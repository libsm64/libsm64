#pragma once

#include <stdint.h>
#include <stddef.h>

#include "decomp/include/types.h"

extern void load_mario_animation(struct MarioAnimation *a, u32 index);
extern void load_mario_anims_from_rom( const uint8_t *rom );
extern void unload_mario_anims( void );