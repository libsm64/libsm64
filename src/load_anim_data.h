#pragma once

#include <stdint.h>
#include <stddef.h>

#include "decomp/include/types.h"

extern struct Animation *g_libsm64_mario_animations;

extern void load_mario_anims_from_rom( uint8_t *rom );
extern void unload_mario_anims( void );