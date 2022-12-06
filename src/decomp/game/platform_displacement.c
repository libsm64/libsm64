#include <math.h>

#include "../engine/math_util.h"
#include "../engine/surface_collision.h"
#include "level_update.h"
#include "../include/object_fields.h"
#include "object_stuff.h"
#include "platform_displacement.h"
#include "../shim.h"
#include "../global_state.h"
#include "../../load_surfaces.h"

#define absfx( x ) ( (x) < 0.0f ? -(x) : (x) )

/**
 * Determine if Mario is standing on a platform object, meaning that he is
 * within 4 units of the floor. Set his referenced platform object accordingly.
 */
void update_mario_platform(void) {
    struct SM64SurfaceCollisionData *floor;
    UNUSED u32 unused;
    f32 marioX;
    f32 marioY;
    f32 marioZ;
    f32 floorHeight;
    u32 awayFromFloor;

    if (gMarioObject == NULL) {
        return;
    }

    //! If Mario moves onto a rotating platform in a PU, the find_floor call
    //  will detect the platform and he will end up receiving a large amount
    //  of displacement since he is considered to be far from the platform's
    //  axis of rotation.

    marioX = gMarioObject->oPosX;
    marioY = gMarioObject->oPosY;
    marioZ = gMarioObject->oPosZ;
    floorHeight = find_floor(marioX, marioY, marioZ, &floor);

    if (absfx(marioY - floorHeight) < 4.0f) {
        awayFromFloor = 0;
    } else {
        awayFromFloor = 1;
    }

    switch (awayFromFloor) {
        case 1:
            gMarioObject->platform = NULL;
            break;

        case 0:
            if (floor != NULL && floor->transform != NULL) {
                gMarioObject->platform = floor->transform;
            } else {
                gMarioObject->platform = NULL;
            }
            break;
    }
}

/**
 * Get Mario's position and store it in x, y, and z.
 */
static void get_mario_pos(f32 *x, f32 *y, f32 *z) {
    *x = gMarioState->pos[0];
    *y = gMarioState->pos[1];
    *z = gMarioState->pos[2];
}

/**
 * Set Mario's position.
 */
static void set_mario_pos(f32 x, f32 y, f32 z) {
    gMarioState->pos[0] = x;
    gMarioState->pos[1] = y;
    gMarioState->pos[2] = z;
}

/**
 * Apply one frame of platform rotation to Mario or an object using the given
 * platform. If isMario is false, use gCurrentObject.
 */
void apply_platform_displacement(u32 isMario, struct SM64SurfaceObjectTransform *platform) {
    f32 x;
    f32 y;
    f32 z;
    f32 platformPosX;
    f32 platformPosY;
    f32 platformPosZ;
    Vec3f currentObjectOffset;
    Vec3f relativeOffset;
    Vec3f newObjectOffset;
    Vec3s rotation;
    UNUSED s16 unused1;
    UNUSED s16 unused2;
    UNUSED s16 unused3;
    f32 displaceMatrix[4][4];

    rotation[0] = platform->aAngleVelPitch;
    rotation[1] = platform->aAngleVelYaw;
    rotation[2] = platform->aAngleVelRoll;

//  if (isMario) {
//      D_8032FEC0 = 0;
        get_mario_pos(&x, &y, &z);
//  } else {
//      x = gCurrentObject->aPosX;
//      y = gCurrentObject->aPosY;
//      z = gCurrentObject->aPosZ;
//  }

    x += platform->aVelX;
    z += platform->aVelZ;

    if (rotation[0] != 0 || rotation[1] != 0 || rotation[2] != 0) {
        unused1 = rotation[0];
        unused2 = rotation[2];
        unused3 = platform->aFaceAngleYaw;

        if (isMario) {
            gMarioState->faceAngle[1] += rotation[1];
        }

        platformPosX = platform->aPosX;
        platformPosY = platform->aPosY;
        platformPosZ = platform->aPosZ;

        currentObjectOffset[0] = x - platformPosX;
        currentObjectOffset[1] = y - platformPosY;
        currentObjectOffset[2] = z - platformPosZ;

        rotation[0] = platform->aFaceAnglePitch - platform->aAngleVelPitch;
        rotation[1] = platform->aFaceAngleYaw - platform->aAngleVelYaw;
        rotation[2] = platform->aFaceAngleRoll - platform->aAngleVelRoll;

        mtxf_rotate_zxy_and_translate(displaceMatrix, currentObjectOffset, rotation);
        linear_mtxf_transpose_mul_vec3f(displaceMatrix, relativeOffset, currentObjectOffset);

        rotation[0] = platform->aFaceAnglePitch;
        rotation[1] = platform->aFaceAngleYaw;
        rotation[2] = platform->aFaceAngleRoll;

        mtxf_rotate_zxy_and_translate(displaceMatrix, currentObjectOffset, rotation);
        linear_mtxf_mul_vec3f(displaceMatrix, newObjectOffset, relativeOffset);

        x = platformPosX + newObjectOffset[0];
        y = platformPosY + newObjectOffset[1];
        z = platformPosZ + newObjectOffset[2];
    }

//  if (isMario) {
        set_mario_pos(x, y, z);
//  } else {
//      gCurrentObject->oPosX = x;
//      gCurrentObject->oPosY = y;
//      gCurrentObject->oPosZ = z;
//  }
}

/**
 * If Mario's platform is not null, apply platform displacement.
 */
void apply_mario_platform_displacement(void) {
    if (gMarioObject != NULL && gMarioObject->platform != NULL) {
        apply_platform_displacement(TRUE, gMarioObject->platform);
    }
}