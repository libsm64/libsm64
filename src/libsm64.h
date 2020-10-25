#ifndef __LIB_SM64_H
#define __LIB_SM64_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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

struct SM64ObjectTransform
{
    float position[3];
    float eulerRotation[3];
};

struct SM64SurfaceObject
{
    struct SM64ObjectTransform transform;
    uint32_t surfaceCount;
    struct SM64Surface *surfaces;
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
    float *position;
    float *normal;
    float *color;
    float *uv;
    uint16_t numTrianglesUsed;
};

typedef void (*SM64DebugPrintFunctionPtr)( const char * );

enum
{
    SM64_TEXTURE_WIDTH = 64 * 11,
    SM64_TEXTURE_HEIGHT = 64,
    SM64_GEO_MAX_TRIANGLES = 1024,
};

#ifdef _WIN32
    #define SM64_EXPORT __declspec(dllexport)
#else
    #define SM64_EXPORT
#endif

extern SM64_EXPORT void sm64_global_init( uint8_t *rom, uint8_t *outTexture, SM64DebugPrintFunctionPtr debugPrintFunction );
extern SM64_EXPORT void sm64_load_surfaces( uint16_t terrainType, const struct SM64Surface *surfaceArray, uint32_t numSurfaces );

extern SM64_EXPORT void sm64_mario_reset( int16_t marioX, int16_t marioY, int16_t marioZ );
extern SM64_EXPORT void sm64_mario_tick( const struct SM64MarioInputs *inputs, struct SM64MarioState *outState, struct SM64MarioGeometryBuffers *outBuffers );
extern SM64_EXPORT void sm64_global_terminate( void );

extern SM64_EXPORT uint32_t sm64_load_surface_object( const struct SM64SurfaceObject *surfaceObject );
extern SM64_EXPORT void sm64_move_object( uint32_t id, const struct SM64ObjectTransform *transform );
extern SM64_EXPORT void sm64_unload_object( uint32_t id );

/*

extern void sm64_global_init( uint8_t *rom, uint8_t *outTexture, SM64DebugPrintFunctionPtr debugPrintFunction );
extern uint8_t sm64_global_is_init( void );
extern void sm64_global_terminate( void );

extern void sm64_create_static_surfaces( uint16_t terrainType, const struct SM64Surface *surfaceArray, uint32_t numSurfaces );
extern void sm64_delete_static_surfaces( void );

extern uint32_t sm64_create_surface_object( const struct SM64SurfaceObject *surfaceObject );
extern void sm64_move_object( uint32_t objectId, const struct SM64ObjectTransform *transform );
extern void sm64_delete_object( uint32_t objectId );

extern uint32_t sm64_create_mario( int16_t x, int16_t y, int16_t z );
extern void sm64_mario_tick( const struct SM64MarioInputs *inputs, struct SM64MarioState *outState, struct SM64MarioGeometryBuffers *outBuffers );
extern void sm64_delete_mario( uint32_t marioId );

*/

#endif//__LIB_SM64_H