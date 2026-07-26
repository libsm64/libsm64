#include <string.h>
#include "pole.h"
#include "decomp/include/object_fields.h"

static struct Object sPolePool[MAX_POLE_OBJECTS];
static u8            sPoleInUse[MAX_POLE_OBJECTS];

void pole_pool_init(void) {
    memset(sPolePool, 0, sizeof(sPolePool));
    memset(sPoleInUse, 0, sizeof(sPoleInUse));
}

s32 pole_pool_create(f32 x, f32 y, f32 z, f32 height, f32 downOffset, s16 pitch, s16 roll) {
    for (s32 i = 0; i < MAX_POLE_OBJECTS; i++) {
        if (!sPoleInUse[i]) {
            struct Object *o = &sPolePool[i];
            memset(o, 0, sizeof(struct Object));

            o->oPosX = x;
            o->oPosY = y;
            o->oPosZ = z;
            o->oMoveAnglePitch = pitch;
            o->oMoveAngleRoll  = roll;

            o->hitboxHeight     = height;
            o->hitboxDownOffset = downOffset;
            o->behavior = NULL;

            sPoleInUse[i] = 1;
            return i;
        }
    }
    return -1;
}

void pole_pool_release(s32 handle) {
    if (handle < 0 || handle >= MAX_POLE_OBJECTS || !sPoleInUse[handle]) {
        return;
    }
    sPoleInUse[handle] = 0;
}

struct Object *pole_pool_get(s32 handle) {
    if (handle < 0 || handle >= MAX_POLE_OBJECTS || !sPoleInUse[handle]) {
        return NULL;
    }
    return &sPolePool[handle];
}