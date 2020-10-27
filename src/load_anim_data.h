#pragma once

#include <stdint.h>
#include <stddef.h>

#include "decomp/include/types.h"

extern struct Animation *gLibSm64MarioAnimations;

extern void load_mario_anims_from_rom( uint8_t *rom );