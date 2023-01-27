#include "decomp/global_state.h"

#include "debug_print.h"
#include "load_surfaces.h"
#include "gfx_adapter.h"
#include "load_anim_data.h"
#include "load_tex_data.h"
#include "obj_pool.h"

#include "decomp/game/interaction.h"
#include "decomp/include/object_fields.h"
#include "decomp/include/sm64.h"
#include "decomp/shim.h"
#include "decomp/game/mario.h"
#include "decomp/engine/math_util.h"
#include "fake_interaction.h"

#define INT_GROUND_POUND_OR_TWIRL (1 << 0) // 0x01
#define INT_PUNCH                 (1 << 1) // 0x02
#define INT_KICK                  (1 << 2) // 0x04
#define INT_TRIP                  (1 << 3) // 0x08
#define INT_SLIDE_KICK            (1 << 4) // 0x10
#define INT_FAST_ATTACK_OR_SHELL  (1 << 5) // 0x20
#define INT_HIT_FROM_ABOVE        (1 << 6) // 0x40
#define INT_HIT_FROM_BELOW        (1 << 7) // 0x80

#define INT_ATTACK_NOT_FROM_BELOW                                                 \
    (INT_GROUND_POUND_OR_TWIRL | INT_PUNCH | INT_KICK | INT_TRIP | INT_SLIDE_KICK \
     | INT_FAST_ATTACK_OR_SHELL | INT_HIT_FROM_ABOVE)

#define INT_ANY_ATTACK                                                            \
    (INT_GROUND_POUND_OR_TWIRL | INT_PUNCH | INT_KICK | INT_TRIP | INT_SLIDE_KICK \
     | INT_FAST_ATTACK_OR_SHELL | INT_HIT_FROM_ABOVE | INT_HIT_FROM_BELOW)

#define INT_ATTACK_NOT_WEAK_FROM_ABOVE                                                \
    (INT_GROUND_POUND_OR_TWIRL | INT_PUNCH | INT_KICK | INT_TRIP | INT_HIT_FROM_BELOW)

#define sDelayInvincTimer (g_state->msDelayInvincTimer)
#define sInvulnerable     (g_state->msInvulnerable)

static u32 sForwardKnockbackActions[][3] = {
    { ACT_SOFT_FORWARD_GROUND_KB, ACT_FORWARD_GROUND_KB, ACT_HARD_FORWARD_GROUND_KB },
    { ACT_FORWARD_AIR_KB,         ACT_FORWARD_AIR_KB,    ACT_HARD_FORWARD_AIR_KB },
    { ACT_FORWARD_WATER_KB,       ACT_FORWARD_WATER_KB,  ACT_FORWARD_WATER_KB },
};

static u32 sBackwardKnockbackActions[][3] = {
    { ACT_SOFT_BACKWARD_GROUND_KB, ACT_BACKWARD_GROUND_KB, ACT_HARD_BACKWARD_GROUND_KB },
    { ACT_BACKWARD_AIR_KB,         ACT_BACKWARD_AIR_KB,    ACT_HARD_BACKWARD_AIR_KB },
    { ACT_BACKWARD_WATER_KB,       ACT_BACKWARD_WATER_KB,  ACT_BACKWARD_WATER_KB },
};

uint32_t fake_damage_knock_back(struct MarioState *m, uint32_t damage,uint32_t interactionSubtype,float xSrc,float ySrc,float zSrc) {

    if (!sInvulnerable && !(m->flags & MARIO_VANISH_CAP)
        && !(interactionSubtype & INT_SUBTYPE_DELAY_INVINCIBILITY)) {
        //o->oInteractStatus = INT_STATUS_INTERACTED | INT_STATUS_ATTACKED_MARIO;
        //m->interactObj = o;

        // calculate damage, substitute for take_damage_from_interact_object
        if (!(m->flags & MARIO_CAP_ON_HEAD)) {
            damage += (damage + 1) / 2;
        }

        if (m->flags & MARIO_METAL_CAP) {
            damage = 0;
        }

        m->hurtCounter += 4 * damage; // apply hurt counter

        if (interactionSubtype & INT_SUBTYPE_BIG_KNOCKBACK) {
            m->forwardVel = 40.0f;
        }

        if (damage > 0) {
            play_sound(SOUND_MARIO_ATTACKED, m->marioObj->header.gfx.cameraToObject);
        }

        //update_mario_sound_and_camera(m);
        return drop_and_set_mario_action(m, fake_determine_knockback_action(m, damage,xSrc,ySrc,zSrc),
                                         damage);
    }

    return FALSE;
}

s16 fake_mario_obj_angle_to_object(struct MarioState *m, float xSrc,float zSrc) {
    f32 dx = xSrc - m->pos[0];
    f32 dz = zSrc - m->pos[2];

    return atan2s(dz, dx);
}

u32 fake_determine_knockback_action(struct MarioState *m, s32 damage,float xSrc,float ySrc,float zSrc) {
    u32 bonkAction;

    s16 terrainIndex = 0; // 1 = air, 2 = water, 0 = default
    s16 strengthIndex = 0;

    s16 angleToObject = fake_mario_obj_angle_to_object(m, xSrc,zSrc);
    s16 facingDYaw = angleToObject - m->faceAngle[1];
    s16 remainingHealth = m->health - 0x40 * m->hurtCounter;

    if (m->action & (ACT_FLAG_SWIMMING | ACT_FLAG_METAL_WATER)) {
        terrainIndex = 2;
    } else if (m->action & (ACT_FLAG_AIR | ACT_FLAG_ON_POLE | ACT_FLAG_HANGING)) {
        terrainIndex = 1;
    }

    if (remainingHealth < 0x100) {
        strengthIndex = 2;
    } else if (damage >= 4) {
        strengthIndex = 2;
    } else if (damage >= 2) {
        strengthIndex = 1;
    }

    m->faceAngle[1] = angleToObject;

    if (terrainIndex == 2) {
        if (m->forwardVel < 28.0f) {
            mario_set_forward_vel(m, 28.0f);
        }

        if (m->pos[1] >= ySrc) {
            if (m->vel[1] < 20.0f) {
                m->vel[1] = 20.0f;
            }
        } else {
            if (m->vel[1] > 0.0f) {
                m->vel[1] = 0.0f;
            }
        }
    } else {
        if (m->forwardVel < 16.0f) {
            mario_set_forward_vel(m, 16.0f);
        }
    }

    if (-0x4000 <= facingDYaw && facingDYaw <= 0x4000) {
        m->forwardVel *= -1.0f;
        bonkAction = sBackwardKnockbackActions[terrainIndex][strengthIndex];
    } else {
        m->faceAngle[1] += 0x8000;
        bonkAction = sForwardKnockbackActions[terrainIndex][strengthIndex];
    }

    return bonkAction;
}

s16 fake_mario_obj_angle_to_pos(struct MarioState *m, float x, float z) {
    f32 dx = x - m->pos[0];
    f32 dz = z - m->pos[2];

    return atan2s(dz, dx);
}

void fake_bounce_back_from_attack(struct MarioState *m, u32 interaction) {
    if (interaction & (INT_PUNCH | INT_KICK | INT_TRIP)) {
        if (m->action == ACT_PUNCHING) {
            m->action = ACT_MOVE_PUNCHING;
        }

        if (m->action & ACT_FLAG_AIR) {
            mario_set_forward_vel(m, -16.0f);
        } else {
            mario_set_forward_vel(m, -48.0f);
        }

        //set_camera_shake_from_hit(SHAKE_ATTACK);
        m->particleFlags |= PARTICLE_TRIANGLE;
    }

    if (interaction & (INT_PUNCH | INT_KICK | INT_TRIP | INT_FAST_ATTACK_OR_SHELL)) {
        play_sound(SOUND_ACTION_HIT_2, m->marioObj->header.gfx.cameraToObject);
    }
}

u32 fake_determine_interaction(struct MarioState *m, float x, float y, float z) {
    u32 interaction = 0;
    u32 action = m->action;

    s16 dYawToObject = fake_mario_obj_angle_to_pos(m, x, z) - m->faceAngle[1];
    if (action & ACT_FLAG_ATTACKING) {
        if (action == ACT_PUNCHING || action == ACT_MOVE_PUNCHING || action == ACT_JUMP_KICK) {
            

            if (m->flags & MARIO_PUNCHING) {
                // 120 degrees total, or 60 each way
                if (-0x2AAA <= dYawToObject && dYawToObject <= 0x2AAA) {
                    interaction = INT_PUNCH;
                }
            }
            if (m->flags & MARIO_KICKING) {
                // 120 degrees total, or 60 each way
                if (-0x2AAA <= dYawToObject && dYawToObject <= 0x2AAA) {
                    interaction = INT_KICK;
                }
            }
            if (m->flags & MARIO_TRIPPING) {
                // 180 degrees total, or 90 each way
                if (-0x4000 <= dYawToObject && dYawToObject <= 0x4000) {
                    interaction = INT_TRIP;
                }
            }
        } else if (action == ACT_GROUND_POUND || action == ACT_TWIRLING) {
            if (m->vel[1] < 0.0f) {
                interaction = INT_GROUND_POUND_OR_TWIRL;
            }
        } else if (action == ACT_GROUND_POUND_LAND || action == ACT_TWIRL_LAND) {
            // Neither ground pounding nor twirling change Mario's vertical speed on landing.,
            // so the speed check is nearly always true (perhaps not if you land while going upwards?)
            // Additionally, actionState it set on each first thing in their action, so this is
            // only true prior to the very first frame (i.e. active 1 frame prior to it run).
            if (m->vel[1] < 0.0f && m->actionState == 0) {
                interaction = INT_GROUND_POUND_OR_TWIRL;
            }
        } else if (action == ACT_SLIDE_KICK || action == ACT_SLIDE_KICK_SLIDE) {
            interaction = INT_SLIDE_KICK;
        } else if (action & ACT_FLAG_RIDING_SHELL) {
            interaction = INT_FAST_ATTACK_OR_SHELL;
        } else if (m->forwardVel <= -26.0f || 26.0f <= m->forwardVel) {
            interaction = INT_FAST_ATTACK_OR_SHELL;
        }
    }

    // Prior to this, the interaction type could be overwritten. This requires, however,
    // that the interaction not be set prior. This specifically overrides turning a ground
    // pound into just a bounce.
    if (interaction == 0 && (action & ACT_FLAG_AIR)) {
        if (m->vel[1] < 0.0f) {
            if (m->pos[1] > y) {
                interaction = INT_HIT_FROM_ABOVE;
            }
        } else {
            if (m->pos[1] < y) {
                interaction = INT_HIT_FROM_BELOW;
            }
        }
    }

    return interaction;
}

void fake_bounce_off_object(struct MarioState *m, float x, float y, float z, float hitboxHeight, f32 velY) {
    //m->pos[1] = y + hitboxHeight;
    m->vel[1] = velY;

    m->flags &= ~MARIO_UNKNOWN_08;

    play_sound(SOUND_ACTION_BOUNCE_OFF_OBJECT, m->marioObj->header.gfx.cameraToObject);
}

u32 fake_interact_hit_from_below(struct MarioState *m, float x, float y, float z, float hitboxHeight) {

    u32 interaction;
    if (m->flags & MARIO_METAL_CAP) {
        interaction = INT_FAST_ATTACK_OR_SHELL;
    } else {
        interaction = fake_determine_interaction(m, x, y, z);
    }

    if (interaction & INT_ANY_ATTACK) {
        fake_bounce_back_from_attack(m, interaction);

        if (interaction & INT_HIT_FROM_BELOW) {
            m->vel[1] = 0.0f;
        }

        if (interaction & INT_HIT_FROM_ABOVE) {
            fake_bounce_off_object(m, x, y, z, hitboxHeight, 30.0f);
        }
        return TRUE;
    } else/* if (fake_damage_knock_back(m, 0, 0, x, y, z))*/ {
        return FALSE;
    }

    return FALSE;
}

u32 fake_interact_bounce_top(struct MarioState *m, float x, float y, float z, float hitboxHeight) {
    u32 interaction;
    if (m->flags & MARIO_METAL_CAP) {
        interaction = INT_FAST_ATTACK_OR_SHELL;
    } else {
        interaction = fake_determine_interaction(m, x, y, z);
    }

    if (interaction & INT_ATTACK_NOT_FROM_BELOW) {
        fake_bounce_back_from_attack(m, interaction);

        if (interaction & INT_HIT_FROM_ABOVE) {
            fake_bounce_off_object(m, x, y, z, hitboxHeight, 30.0f);
        }
        return TRUE;
    } else/* if (fake_damage_knock_back(m, 0, 0, x, y, z))*/ {
        return FALSE;
    }

    return FALSE;
}