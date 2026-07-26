#ifndef POLE_H
#define POLE_H

#include "decomp/include/types.h"

#define MAX_POLE_OBJECTS 128

void pole_pool_init(void);

s32 pole_pool_create(f32 x, f32 y, f32 z, f32 height, f32 downOffset, s16 pitch, s16 roll);

void pole_pool_release(s32 handle);

struct Object *pole_pool_get(s32 handle);

#endif // POLE_H