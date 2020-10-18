#pragma once

#include "include/types.h"
#include "game/area.h"
#include "game/level_update.h"
#include "libsm64.h"

#define COURSE_MIN  0
#define COURSE_MAX 14
#define LEVEL_LLL  99
#define LEVEL_SSL  98
#define LEVEL_DDD  97
#define LEVEL_TTC  96
#define LEVEL_CASTLE 95
#define LEVEL_THI  94

// From graph_node.h, trying not to pull that in yet
#define GRAPH_RENDER_INVISIBLE (1 << 4)

#define play_sound(a,b) ({})
#define enable_time_stop() ({})
#define disable_time_stop() ({})
#define play_cutscene_music(a) ({})

struct SurfaceObjectTransform
{
    float aPosX, aPosY, aPosZ;
    float aVelX, aVelY, aVelZ;

    s16 aFaceAnglePitch;
    s16 aFaceAngleYaw;
    s16 aFaceAngleRoll;

    s16 aAngleVelPitch;
    s16 aAngleVelYaw;
    s16 aAngleVelRoll;
};

struct SurfaceNode
{
    struct SurfaceNode *next;
    struct Surface *surface;
};

extern u32 gGlobalTimer;
extern u8 gSpecialTripleJump;
extern struct HudDisplay gHudDisplay;
extern s16 gCurrLevelNum;
extern s16 gCameraMovementFlags;
extern u32 gAudioRandom;
extern s8 gShowDebugText;
extern s8 gDebugLevelSelect;
extern s16 gCurrSaveFileNum;
extern struct Controller gController;
extern struct SpawnInfo gMarioSpawnInfoVal;
extern struct SpawnInfo *gMarioSpawnInfo;
extern struct Area *gCurrentArea;
extern struct Object *gCurrentObject;
extern struct Object *gMarioObject;
extern struct PlayerCameraState gPlayerCameraState;
extern struct MarioAnimation D_80339D10;
extern struct MarioState *gMarioState;
extern SM64DebugPrintFunctionPtr gDebugPrint;

#define DEBUG_LOG( ... ) do { \
    if( gDebugPrint ) { \
        char debugStr[1024]; \
        sprintf( debugStr, __VA_ARGS__ ); \
        gDebugPrint( debugStr ); \
    } \
} while(0)

extern void hack_load_mario_animation_ex(struct MarioAnimation *a, u32 index);
extern void hack_load_mario_animation(struct MarioAnimation *a, u32 index);
extern void *segmented_to_virtual(const void *addr);
extern void *virtual_to_segmented(u32 segment, const void *addr);
extern void func_80320A4C(u8 bankIndex, u8 arg1);
extern void raise_background_noise(s32 a);
extern void lower_background_noise(s32 a);
extern void set_camera_mode(struct Camera *c, s16 mode, s16 frames);
extern void print_text_fmt_int(s32 x, s32 y, const char *str, s32 n);
extern s16 level_trigger_warp(struct MarioState *m, s32 warpOp);
extern void play_cap_music(u16 seqArgs);
extern void fadeout_cap_music(void);
extern void stop_cap_music(void);
extern void play_infinite_stairs_music(void);
extern s32 save_file_get_total_star_count(s32 fileIndex, s32 minCourse, s32 maxCourse);
extern u32 save_file_get_flags(void);
extern void save_file_set_flags(u32 flags);
extern void save_file_clear_flags(u32 flags);
extern void spawn_wind_particles(s16 pitch, s16 yaw);
extern void set_camera_shake_from_hit(s16 shake);
extern void load_level_init_text(u32 arg);
extern void spawn_default_star(f32 sp20, f32 sp24, f32 sp28);
extern void play_shell_music(void);
extern void stop_shell_music(void);
extern u16 level_control_timer(s32 timerOp);