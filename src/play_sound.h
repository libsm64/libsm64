#pragma once

#include "decomp/include/types.h"
#include "libsm64.h"
#include <stdio.h>

extern SM64PlaySoundFunctionPtr g_play_sound_func;

extern void play_sound( uint32_t soundBits, f32 *pos );