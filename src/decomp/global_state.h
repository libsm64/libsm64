#pragma once

#include "include/types.h"
#include "game/area.h"

struct GlobalState
{
    // interaction.c
    u8 msDelayInvincTimer;
    s16 msInvulnerable;

    // mario_actions_moving.c
    Mat4 msFloorAlignMatrix;

    // mario_actions_submerged.c
    s16 msWasAtSurface;
    s16 msSwimStrength;
    s16 mD_80339FD0;
    s16 mD_80339FD2;
    f32 mD_80339FD4;

    // mario_misc.c
    struct MarioBodyState mgBodyStates[2];
    s16 msMarioAttackAnimCounter;

    // rendering_graph_node.c
    u16 mgAreaUpdateCounter;

    // misc
    u32 mgGlobalTimer;
    u8 mgSpecialTripleJump;
    s16 mgCurrLevelNum;
    s16 mgCameraMovementFlags;
    u32 mgAudioRandom;
    s8 mgShowDebugText;
    s8 mgDebugLevelSelect;
    s16 mgCurrSaveFileNum;
    struct Controller mgController;
    struct SpawnInfo mgMarioSpawnInfoVal;
    struct Area *mgCurrentArea;
    struct Object *mgCurrentObject;
    struct Object *mgMarioObject;
    struct MarioAnimation mD_80339D10;
    struct MarioState mgMarioStateVal;
};

// From mario_actions_submerged.c, needed to initialize global state
#define MIN_SWIM_STRENGTH 160

extern struct GlobalState *g_state;

extern struct GlobalState *global_state_create(void);
extern void global_state_bind(struct GlobalState *state);
extern void global_state_delete(struct GlobalState *state);