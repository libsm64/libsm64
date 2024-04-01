/**
 * This file just hacks a bunch of stuff together to get a valid Object and GraphNode for Mario
 */
#include "object_stuff.h"
#include <stdlib.h>
#include "mario.h"
#include "../shim.h"
#include "../engine/math_util.h"
#include "../include/object_fields.h"

static Vec3f gVec3fZero = { 0.0f, 0.0f, 0.0f };
static Vec3s gVec3sZero = { 0, 0, 0 };

static struct Object *try_allocate_object(void) {
    struct ObjectNode *nextObj;
    nextObj = (struct ObjectNode *) malloc(sizeof(struct Object));
    nextObj->prev = NULL;
    nextObj->next = NULL;
    return (struct Object *) nextObj;
}

static struct Object *allocate_object(void) {
    s32 i;
    struct Object *obj = try_allocate_object();

    // Initialize object fields

    obj->activeFlags = ACTIVE_FLAG_ACTIVE | ACTIVE_FLAG_UNK8;
    obj->parentObj = obj;
    obj->prevObj = NULL;
    obj->collidedObjInteractTypes = 0;
    obj->numCollidedObjs = 0;

    for (i = 0; i < 0x50; i++) {
#ifdef _WIN32
        obj->rawData.asS32[i] = 0;
#endif
#ifdef _WIN64
        obj->ptrData.asVoidPtr[i] = NULL;
#endif
    }

    obj->unused1 = 0;
    obj->bhvStackIndex = 0;
    obj->bhvDelayTimer = 0;

    obj->hitboxRadius = 37.0f;    // Override directly for Mario
    obj->hitboxHeight = 160.0f;   //
    obj->hurtboxRadius = 0.0f;
    obj->hurtboxHeight = 0.0f;
    obj->hitboxDownOffset = 0.0f;
    obj->unused2 = 0;

    obj->platform = NULL;
    obj->collisionData = NULL;
    obj->oIntangibleTimer = -1;
    obj->oDamageOrCoinValue = 0;
    obj->oHealth = 2048;

    obj->oCollisionDistance = 1000.0f;
    if (gCurrLevelNum == LEVEL_TTC) {
        obj->oDrawingDistance = 2000.0f;
    } else {
        obj->oDrawingDistance = 4000.0f;
    }

    mtxf_identity(obj->transform);

    obj->respawnInfoType = RESPAWN_INFO_TYPE_NULL;
    obj->respawnInfo = NULL;

    obj->oDistanceToMario = 19000.0f;
    obj->oRoom = -1;

    obj->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;
    obj->header.gfx.pos[0] = -10000.0f;
    obj->header.gfx.pos[1] = -10000.0f;
    obj->header.gfx.pos[2] = -10000.0f;
    obj->header.gfx.throwMatrix = NULL;

    return obj;
}

static struct Object *create_object(void) {
    struct Object *obj;
    obj = allocate_object();
    obj->curBhvCommand = NULL;
    obj->behavior = NULL;
    return obj;
}

static void geo_obj_init(struct GraphNodeObject *graphNode, void *sharedChild, Vec3f pos, Vec3s angle) {
    vec3f_set(graphNode->scale, 1.0f, 1.0f, 1.0f);
    vec3f_copy(graphNode->pos, pos);
    vec3s_copy(graphNode->angle, angle);

    graphNode->sharedChild = sharedChild;
    graphNode->unk4C = 0;
    graphNode->throwMatrix = NULL;
    graphNode->animInfo.curAnim = NULL;

    graphNode->node.flags |= GRAPH_RENDER_ACTIVE;
    graphNode->node.flags &= ~GRAPH_RENDER_INVISIBLE;
    graphNode->node.flags |= GRAPH_RENDER_HAS_ANIMATION;
    graphNode->node.flags &= ~GRAPH_RENDER_BILLBOARD;
}

static struct Object *spawn_object_at_origin(void) {
    struct Object *obj;
    obj = create_object();

    obj->parentObj = NULL;
    obj->header.gfx.areaIndex = 0;
    obj->header.gfx.activeAreaIndex = 0;
    geo_obj_init((struct GraphNodeObject *) &obj->header.gfx, NULL, gVec3fZero, gVec3sZero);

    return obj;
}

/**
 * Copy position, velocity, and angle variables from MarioState to the Mario
 * object.
 */
static void copy_mario_state_to_object(void) {
    s32 i = 0;
    // L is real
    if (gCurrentObject != gMarioObject) {
        i += 1;
    }

    gCurrentObject->oVelX = gMarioState->vel[0];
    gCurrentObject->oVelY = gMarioState->vel[1];
    gCurrentObject->oVelZ = gMarioState->vel[2];

    gCurrentObject->oPosX = gMarioState->pos[0];
    gCurrentObject->oPosY = gMarioState->pos[1];
    gCurrentObject->oPosZ = gMarioState->pos[2];

    gCurrentObject->oMoveAnglePitch = gCurrentObject->header.gfx.angle[0];
    gCurrentObject->oMoveAngleYaw = gCurrentObject->header.gfx.angle[1];
    gCurrentObject->oMoveAngleRoll = gCurrentObject->header.gfx.angle[2];

    gCurrentObject->oFaceAnglePitch = gCurrentObject->header.gfx.angle[0];
    gCurrentObject->oFaceAngleYaw = gCurrentObject->header.gfx.angle[1];
    gCurrentObject->oFaceAngleRoll = gCurrentObject->header.gfx.angle[2];

    gCurrentObject->oAngleVelPitch = gMarioState->angleVel[0];
    gCurrentObject->oAngleVelYaw = gMarioState->angleVel[1];
    gCurrentObject->oAngleVelRoll = gMarioState->angleVel[2];
}

struct Object *hack_allocate_mario(void)
{
    return spawn_object_at_origin();
}

/**
 * Mario's primary behavior update function.
 */
void bhv_mario_update(void) {
    u32 particleFlags = 0;
//  s32 i;

	gCurrentObject = gMarioObject;
    particleFlags = execute_mario_action(gCurrentObject);
    gCurrentObject->oMarioParticleFlags = particleFlags;

    // Mario code updates MarioState's versions of position etc, so we need
    // to sync it with the Mario object
    copy_mario_state_to_object();

//  i = 0;
//  while (sParticleTypes[i].particleFlag != 0) {
//      if (particleFlags & sParticleTypes[i].particleFlag) {
//          spawn_particle(sParticleTypes[i].activeParticleFlag, sParticleTypes[i].model,
//                         sParticleTypes[i].behavior);
//      }

//      i++;
//  }
}

void create_transformation_from_matrices(Mat4 a0, Mat4 a1, Mat4 a2) {
    f32 spC, sp8, sp4;

    spC = a2[3][0] * a2[0][0] + a2[3][1] * a2[0][1] + a2[3][2] * a2[0][2];
    sp8 = a2[3][0] * a2[1][0] + a2[3][1] * a2[1][1] + a2[3][2] * a2[1][2];
    sp4 = a2[3][0] * a2[2][0] + a2[3][1] * a2[2][1] + a2[3][2] * a2[2][2];

    a0[0][0] = a1[0][0] * a2[0][0] + a1[0][1] * a2[0][1] + a1[0][2] * a2[0][2];
    a0[0][1] = a1[0][0] * a2[1][0] + a1[0][1] * a2[1][1] + a1[0][2] * a2[1][2];
    a0[0][2] = a1[0][0] * a2[2][0] + a1[0][1] * a2[2][1] + a1[0][2] * a2[2][2];

    a0[1][0] = a1[1][0] * a2[0][0] + a1[1][1] * a2[0][1] + a1[1][2] * a2[0][2];
    a0[1][1] = a1[1][0] * a2[1][0] + a1[1][1] * a2[1][1] + a1[1][2] * a2[1][2];
    a0[1][2] = a1[1][0] * a2[2][0] + a1[1][1] * a2[2][1] + a1[1][2] * a2[2][2];

    a0[2][0] = a1[2][0] * a2[0][0] + a1[2][1] * a2[0][1] + a1[2][2] * a2[0][2];
    a0[2][1] = a1[2][0] * a2[1][0] + a1[2][1] * a2[1][1] + a1[2][2] * a2[1][2];
    a0[2][2] = a1[2][0] * a2[2][0] + a1[2][1] * a2[2][1] + a1[2][2] * a2[2][2];

    a0[3][0] = a1[3][0] * a2[0][0] + a1[3][1] * a2[0][1] + a1[3][2] * a2[0][2] - spC;
    a0[3][1] = a1[3][0] * a2[1][0] + a1[3][1] * a2[1][1] + a1[3][2] * a2[1][2] - sp8;
    a0[3][2] = a1[3][0] * a2[2][0] + a1[3][1] * a2[2][1] + a1[3][2] * a2[2][2] - sp4;

    a0[0][3] = 0;
    a0[1][3] = 0;
    a0[2][3] = 0;
    a0[3][3] = 1.0f;
}
void obj_update_pos_from_parent_transformation(Mat4 a0, struct Object *a1) {
    f32 spC = a1->oParentRelativePosX;
    f32 sp8 = a1->oParentRelativePosY;
    f32 sp4 = a1->oParentRelativePosZ;

    a1->oPosX = spC * a0[0][0] + sp8 * a0[1][0] + sp4 * a0[2][0] + a0[3][0];
    a1->oPosY = spC * a0[0][1] + sp8 * a0[1][1] + sp4 * a0[2][1] + a0[3][1];
    a1->oPosZ = spC * a0[0][2] + sp8 * a0[1][2] + sp4 * a0[2][2] + a0[3][2];
}
void obj_set_gfx_pos_from_pos(struct Object *obj) {
    obj->header.gfx.pos[0] = obj->oPosX;
    obj->header.gfx.pos[1] = obj->oPosY;
    obj->header.gfx.pos[2] = obj->oPosZ;
}