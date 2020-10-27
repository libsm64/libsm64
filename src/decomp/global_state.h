#pragma once

#include "include/types.h"
#include "game/area.h"

struct GlobalState
{
    // interaction.c
    u8 sDelayInvincTimer;
    s16 sInvulnerable;

    // mario_actions_moving.c
    Mat4 sFloorAlignMatrix;

    // mario_actions_submerged.c
    s16 sWasAtSurface;
    s16 sSwimStrength;
    s16 D_80339FD0;
    s16 D_80339FD2;
    f32 D_80339FD4;

    // mario_misc.c
    struct MarioBodyState gBodyStates[2];

    // platform_displacement.c
    void *gMarioPlatform;

    // misc
    u32 gGlobalTimer;
    u8 gSpecialTripleJump;
    s16 gCurrLevelNum;
    s16 gCameraMovementFlags;
    u32 gAudioRandom;
    s8 gShowDebugText;
    s8 gDebugLevelSelect;
    s16 gCurrSaveFileNum;
    struct Controller gController;
    struct SpawnInfo gMarioSpawnInfoVal;
    struct SpawnInfo *gMarioSpawnInfo;
    struct Area *gCurrentArea;
    struct Object *gCurrentObject;
    struct Object *gMarioObject;
    struct MarioAnimation D_80339D10;
    struct MarioState gMarioStateVal;
    struct MarioState *gMarioState;
};

extern struct GlobalState *gState;