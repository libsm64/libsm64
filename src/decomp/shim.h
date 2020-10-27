#pragma once

#include "include/types.h"
#include "game/area.h"
#include "game/level_update.h"
#include "../libsm64.h"
#include "global_state.h"

#define COURSE_MIN    0
#define COURSE_MAX   14
#define LEVEL_LLL    99
#define LEVEL_SSL    98
#define LEVEL_DDD    97
#define LEVEL_TTC    96
#define LEVEL_CASTLE 95
#define LEVEL_THI    94

#define play_sound(a,b)                      {}
#define enable_time_stop()                   {}
#define disable_time_stop()                  {}
#define play_cutscene_music(a)               {}
#define segmented_to_virtual(addr)           ((void*)(addr))
#define virtual_to_segmented(seg,addr)       ((void*)(addr))
#define func_80320A4C(bankIndex, arg1)       {}
#define raise_background_noise(a)            {}
#define lower_background_noise(a)            {}
#define set_camera_mode(c, mode, frames)     {}
#define print_text_fmt_int(x, y, str, n)     {}
#define level_trigger_warp(m, warpOp)        0
#define play_cap_music(seqArgs)              {}
#define fadeout_cap_music()                  {}
#define stop_cap_music()                     {}
#define play_infinite_stairs_music()         {}
#define spawn_wind_particles(pitch, yaw)     {}
#define set_camera_shake_from_hit(shake)     {}
#define load_level_init_text(arg)            {}
#define spawn_default_star(sp20, sp24, sp28) {}
#define play_shell_music()                   {}
#define stop_shell_music()                   {}
#define level_control_timer(timerOp)         0
#define save_file_get_flags()                0
#define save_file_set_flags(flags)           {}
#define save_file_clear_flags(flags)         {}
#define save_file_get_total_star_count(fileIndex, minCourse, maxCourse) 0

#define gGlobalTimer         (g_state->mgGlobalTimer)
#define gSpecialTripleJump   (g_state->mgSpecialTripleJump)
#define gCurrLevelNum        (g_state->mgCurrLevelNum)
#define gCameraMovementFlags (g_state->mgCameraMovementFlags)
#define gAudioRandom         (g_state->mgAudioRandom)
#define gShowDebugText       (g_state->mgShowDebugText)
#define gDebugLevelSelect    (g_state->mgDebugLevelSelect)
#define gCurrSaveFileNum     (g_state->mgCurrSaveFileNum)
#define gController          (g_state->mgController)
#define gMarioSpawnInfoVal   (g_state->mgMarioSpawnInfoVal)
#define gMarioSpawnInfo      (&g_state->mgMarioSpawnInfoVal)
#define gCurrentArea         (g_state->mgCurrentArea)
#define gCurrentObject       (g_state->mgCurrentObject)
#define gMarioObject         (g_state->mgMarioObject)
#define D_80339D10           (g_state->mD_80339D10)
#define gMarioState          (&g_state->mgMarioStateVal)