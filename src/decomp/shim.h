#pragma once

#include "include/types.h"
#include "game/area.h"
#include "game/level_update.h"
#include "../libsm64.h"
#include "../play_sound.h"
#include "global_state.h"

#define COURSE_MIN    0
#define COURSE_MAX   14
#define LEVEL_LLL    99
#define LEVEL_SSL    98
#define LEVEL_DDD    97
#define LEVEL_TTC    96
#define LEVEL_CASTLE 95
#define LEVEL_THI    94

#define gGlobalTimer         (g_state->mgGlobalTimer)
#define gSpecialTripleJump   (g_state->mgSpecialTripleJump)
#define gCurrLevelNum        (g_state->mgCurrLevelNum)
#define gCameraMovementFlags (g_state->mgCameraMovementFlags)
//#define gAudioRandom         (g_state->mgAudioRandom)
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
#define gAreaUpdateCounter   (g_state->mgAreaUpdateCounter)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

static void enable_time_stop() {}
static void disable_time_stop() {}
static void *segmented_to_virtual(const void *addr) { return (void*)addr; }
static void *virtual_to_segmented(u32 segment, const void *addr) { return (void*)addr; }
static void func_80320A4C(u8 bankIndex, u8 arg1) {}
static void raise_background_noise(s32 a) {}
static void lower_background_noise(s32 a) {}
static void set_camera_mode(struct Camera *c, s16 mode, s16 frames) {}
static void print_text_fmt_int(s32 x, s32 y, const char *str, s32 n) {}
static s16 level_trigger_warp(struct MarioState *m, s32 warpOp) { return 0; }
//static void play_cap_music(u16 seqArgs) {}
//static void fadeout_cap_music(void) {}
//static void stop_cap_music(void) {}
static void play_infinite_stairs_music(void) {}
static s32 save_file_get_total_star_count(s32 fileIndex, s32 minCourse, s32 maxCourse) { return 0; }
static u32 save_file_get_flags(void) { return 0; }
static void save_file_set_flags(u32 flags) {}
static void save_file_clear_flags(u32 flags) {}
static void spawn_wind_particles(s16 pitch, s16 yaw) {}
static void set_camera_shake_from_hit(s16 shake) {}
static void load_level_init_text(u32 arg) {}
static void spawn_default_star(f32 sp20, f32 sp24, f32 sp28) {}
static void play_shell_music(void) {}
static void stop_shell_music(void) {}
static u16 level_control_timer(s32 timerOp) { return 0; }

#pragma GCC diagnostic pop