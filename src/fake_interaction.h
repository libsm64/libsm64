#pragma once
#include "decomp/include/types.h"

u32 fake_determine_knockback_action(struct MarioState *m, s32 damage,float xSrc,float ySrc,float zSrc);
s16 fake_mario_obj_angle_to_object(struct MarioState *m, float xSrc,float zSrc);
uint32_t fake_damage_knock_back(struct MarioState *m, uint32_t damage,uint32_t interactionSubtype,float xSrc,float ySrc,float zSrc);
u32 fake_interact_hit_from_below(struct MarioState *m, float x, float y, float z, float hitboxHeight);
u32 fake_interact_bounce_top(struct MarioState *m, float x, float y, float z, float hitboxHeight);