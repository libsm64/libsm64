#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "../src/libsm64.h"

#ifdef __cplusplus
}
#endif

#include "context.h"
#include "cglm.h"

typedef struct CollisionMesh
{
    size_t num_vertices;
    float *position;
    float *normal;
    float *color;
    uint16_t *index;

    GLuint vao;
    GLuint position_buffer;
    GLuint normal_buffer;
}
CollisionMesh;

typedef struct MarioMesh
{
    size_t num_vertices;
    uint16_t *index;

    GLuint vao;
    GLuint position_buffer;
    GLuint normal_buffer;
    GLuint color_buffer;
    GLuint uv_buffer;
}
MarioMesh;

typedef struct RenderState
{
    CollisionMesh collision;
    MarioMesh mario;
    GLuint world_shader;
    GLuint mario_shader;
    GLuint mario_texture;
}
RenderState;


struct Renderer
{
	void (*init)(RenderState *renderState, uint8_t *marioTexture);
	void (*draw)(RenderState *renderState, const vec3 camPos, const struct SM64MarioState *marioState, struct SM64MarioGeometryBuffers *marioGeo);
};

#endif
