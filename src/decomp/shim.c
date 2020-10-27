#include <string.h>
#include "shim.h"
#include "../load_anim_data.h"

u32 gGlobalTimer = 0;
u8 gSpecialTripleJump = FALSE;
s16 gCurrLevelNum = 0;
s16 gCameraMovementFlags = 0;
u32 gAudioRandom = 0;
s8 gShowDebugText = 0;
s8 gDebugLevelSelect = 0;
s16 gCurrSaveFileNum = 1;
struct Controller gController;
struct SpawnInfo gMarioSpawnInfoVal;
struct SpawnInfo *gMarioSpawnInfo = &gMarioSpawnInfoVal;
struct Area *gCurrentArea = NULL;
struct Object *gCurrentObject;
struct Object *gMarioObject = NULL;
struct PlayerCameraState gPlayerCameraState;
struct MarioAnimation D_80339D10;
struct MarioState gMarioStateVal;
struct MarioState *gMarioState = &gMarioStateVal;
SM64DebugPrintFunctionPtr gDebugPrint = NULL;

void hack_load_mario_animation_ex(struct MarioAnimation *a, u32 index)
{
    if ((u32)a->currentAnimAddr == 1 + index)
        return;

    a->currentAnimAddr = (u8*)(1 + index);
    a->targetAnim = &gLibSm64MarioAnimations[index];
}

void hack_load_mario_animation(struct MarioAnimation *a, u32 index)
{
//    struct MarioAnimDmaRelatedThing *sp20 = a->animDmaTable;
//    u8 *addr;
//    u32 size;
//
//    if (index < sp20->count) {
//        addr = sp20->srcAddr + sp20->anim[index].offset;
//        size = sp20->anim[index].size;
//
//        if (a->currentAnimAddr != addr) {
//            u32 a0 = sp20->anim[index].offset;
//            u32 b0 = sp20->anim[index].size;
//
//            memcpy( (u8*)a->targetAnim, (u8*)sp20 + a0, b0 );
//            a->currentAnimAddr = addr;
//
//            struct Animation *targetAnim = a->targetAnim;
//            targetAnim->values =(void*)( (u8 *)targetAnim + (uintptr_t)targetAnim->values );
//            targetAnim->index = (void*)( (u8 *)targetAnim + (uintptr_t)targetAnim->index  );
//        }
//    }
}

void *segmented_to_virtual(const void *addr)
{
    return (void*)addr;
}

void *virtual_to_segmented(u32 segment, const void *addr)
{
    return (void*)addr;
}

void func_80320A4C(u8 bankIndex, u8 arg1)
{
}

void lower_background_noise(s32 a)
{
}

void raise_background_noise(s32 a)
{
}

void set_camera_mode(struct Camera *c, s16 mode, s16 frames)
{
}

void print_text_fmt_int(s32 x, s32 y, const char *str, s32 n)
{
}

s16 level_trigger_warp(struct MarioState *m, s32 warpOp)
{
    return 0;
}

void play_cap_music(u16 seqArgs)
{
}

void fadeout_cap_music(void)
{
}

void stop_cap_music(void)
{
}

void play_infinite_stairs_music(void)
{
}

s32 save_file_get_total_star_count(s32 fileIndex, s32 minCourse, s32 maxCourse)
{
    return 0;
}

u32 save_file_get_flags(void)
{
    return 0;
}

void save_file_set_flags(u32 flags)
{
}

void save_file_clear_flags(u32 flags)
{
}

void spawn_wind_particles(s16 pitch, s16 yaw)
{
}

void set_camera_shake_from_hit(s16 shake)
{
}

void load_level_init_text(u32 arg)
{
}

void spawn_default_star(f32 sp20, f32 sp24, f32 sp28)
{
}

void play_shell_music(void)
{
}

void stop_shell_music(void) 
{
}

u16 level_control_timer(s32 timerOp)
{
}