#ifndef __LIB_SM64_H
#define __LIB_SM64_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define SM64_GEO_BUFFER_SIZE 9216 // 1024 triangles * 9 floats per triangle

struct SM64Surface
{
    int16_t type;
    int16_t force;
    int16_t vertices[3][3];
};

struct SM64MarioInputs
{
    float camLookX, camLookZ;
    float stickX, stickY;
    uint8_t buttonA, buttonB, buttonZ;
};

struct SM64MarioState
{
    float position[3];
    float velocity[3];
    float faceAngle;
    int16_t health;
};

struct SM64MarioGeometryBuffers
{
    size_t bufferMaxSize;
    size_t bufferUsedSize;
    float *position;
    float *normal;
    float *color;
    float *uv;
};

typedef void (*SM64DebugPrintFunctionPtr)( const char * );

extern void sm64_global_init( uint8_t *rom, SM64DebugPrintFunctionPtr debugPrintFunction );
extern uint8_t *sm64_get_texture( void );
extern void sm64_load_surfaces( uint16_t terrainType, const struct SM64Surface *surfaceArray, size_t numSurfaces );
extern void sm64_mario_reset( int16_t marioX, int16_t marioY, int16_t marioZ );
extern void sm64_mario_tick( const struct SM64MarioInputs *inputs, struct SM64MarioState *outState, struct SM64MarioGeometryBuffers *outBuffers );
extern void sm64_global_terminate( void );

#endif//__LIB_SM64_H