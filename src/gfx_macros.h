#pragma once

#include "decomp/include/types.h"
#include "gfx_adapter_commands.h"

/*
 * Vertex (set up for use with colors)
 */
typedef struct {
#ifndef GBI_FLOATS
    short ob[3]; /* x, y, z */
#else
    float ob[3]; /* x, y, z */
#endif
    unsigned short flag;
    short tc[2]; /* texture coord */
    unsigned char cn[4]; /* color & alpha */
} Vtx_t;

/*
 * Vertex (set up for use with normals)
 */
typedef struct {
#ifndef GBI_FLOATS
    short ob[3]; /* x, y, z */
#else
    float ob[3]; /* x, y, z */
#endif
    unsigned short flag;
    short tc[2]; /* texture coord */
    signed char n[3]; /* normal */
    unsigned char a; /* alpha */
} Vtx_tn;

typedef union {
    Vtx_t v; /* Use this one for colors */
    Vtx_tn n; /* Use this one for normals */
    long long int force_structure_alignment;
} Vtx;

typedef struct {
    unsigned char col[3]; /* diffuse light value (rgba) */
    char pad1;
    unsigned char colc[3]; /* copy of diffuse light value (rgba) */
    char pad2;
    signed char dir[3]; /* direction of light (normalized) */
    char pad3;
} Light_t;

typedef struct {
    unsigned char col[3]; /* ambient light value (rgba) */
    char pad1;
    unsigned char colc[3]; /* copy of ambient light value (rgba) */
    char pad2;
} Ambient_t;

typedef union {
    Light_t l;
    long long int force_structure_alignment[2];
} Light;

typedef union {
    Ambient_t l;
    long long int force_structure_alignment[1];
} Ambient;

typedef struct {
    Ambient a;
    Light l[1];
} Lights1;

#define	G_TX_RENDERTILE	      0
#define	G_ON                  1
#define	G_OFF                 0
#define	G_TEXTURE_IMAGE_FRAC  2

typedef intptr_t Gfx;

#define gdSPDefLights1(ar,ag,ab,r1,g1,b1,x1,y1,z1) {{{ {ar,ag,ab},0,{ar,ag,ab},0}}, {{{ {r1,g1,b1},0,{r1,g1,b1},0,{x1,y1,z1},0}}} }

#define gsSPVertex(v, n, v0) \
    GFXCMD_VertexData, \
    (intptr_t)v, n, v0

#define gsSP2Triangles(v00, v01, v02, flag0, v10, v11, v12, flag1) \
    GFXCMD_Triangle, \
    v00, v01, v02, flag0, \
    GFXCMD_Triangle, \
    v10, v11, v12, flag1

#define gsSP1Triangle(v00, v01, v02, flag0) \
    GFXCMD_Triangle, \
    v00, v01, v02, flag0

#define gsSPEndDisplayList() \
    GFXCMD_EndDisplayList

#define gsSPDisplayList(dl) \
    GFXCMD_SubDisplayList, \
    (intptr_t)dl

#define gsSPLight(l, n) \
    GFXCMD_Light, \
    (intptr_t)l, n

#define gsSPTexture(s, t, level, tile, on) \
    GFXCMD_Texture, \
    s, t, on

#define gsDPSetTextureImage(f, s, w, i) \
    GFXCMD_SetTextureImage, \
    i

#define gsDPSetTileSize(t, uls, ult, lrs, lrt) \
    GFXCMD_SetTileSize, \
    uls, ult, lrs, lrt

#define gsDPPipeSync() (GFXCMD_None)
#define gsDPSetCombineMode(a, b) (GFXCMD_None)
#define gsSPSetGeometryMode(word) (GFXCMD_None)
#define gsDPLoadTextureBlock(timg, fmt, siz, width, height, pal, cms, cmt, masks, maskt, shifts, shiftt) (GFXCMD_None)
#define gsSPClearGeometryMode(word) (GFXCMD_None)
#define gsDPSetEnvColor(r, g, b, a) (GFXCMD_None)
#define gsDPSetAlphaCompare(type) (GFXCMD_None)
#define gsDPTileSync() (GFXCMD_None)
#define gsDPSetTile(fmt, siz, line, tmem, tile, palette, cmt, maskt, shiftt, cms, masks, shifts) (GFXCMD_None)
#define gsDPLoadBlock(tile, uls, ult, lrs, dxt) (GFXCMD_None)
#define gsDPLoadSync() (GFXCMD_None)