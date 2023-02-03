#ifndef SURFACE_COLLISION_H
#define SURFACE_COLLISION_H

#include "../include/PR/ultratypes.h"

#include "../../libsm64.h"
#include "../include/types.h"

#define LEVEL_BOUNDARY_MAX  0x2000
#define CELL_SIZE           0x400       

#define CELL_HEIGHT_LIMIT   100000.f
#define FLOOR_LOWER_LIMIT  -110000.f

s32 f32_find_wall_collision(f32 *xPtr, f32 *yPtr, f32 *zPtr, f32 offsetY, f32 radius);
s32 find_wall_collisions(struct SM64WallCollisionData *colData);
f32 find_ceil(f32 posX, f32 posY, f32 posZ, struct SM64SurfaceCollisionData **pceil);
f32 find_floor_height_and_data(f32 xPos, f32 yPos, f32 zPos, struct SM64FloorCollisionData **floorGeo);
f32 find_floor_height(f32 x, f32 y, f32 z);
f32 find_floor(f32 xPos, f32 yPos, f32 zPos, struct SM64SurfaceCollisionData **pfloor);
f32 find_water_level(f32 x, f32 z);
f32 find_poison_gas_level(f32 x, f32 z);

#endif // SURFACE_COLLISION_H
