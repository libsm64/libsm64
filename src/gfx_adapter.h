#pragma once

#include "engine/graph_node.h"
#include "libsm64.h"

// Commented out in gbi.h - Replaced here 
#define gDPPipeSync(pkt) ({})
#define gSPSetGeometryMode(pkt, word) ({})
#define gDPSetRenderMode(pkt, c0, c1) ({})
#define gSPClearGeometryMode(pkt, word) ({})
#define gSPPerspNormalize(pkt, s) ({})
#define gDPFillRectangle(pkt, ulx, uly, lrx, lry) ({})
#define gSPViewport(pkt, v) ({})
extern void gSPMatrix( void *pkt, Mtx *m, uint8_t flags );
extern void gSPDisplayList( void *pkt, struct DisplayListNode *dl );

// from lib/src/gu*.c
#define guOrtho(mtx, left, right, bottom, top, a, b, c) ({})
#define guPerspective(mtx, perspNorm, fov, aspect, near, far, a) ({})

extern void gfx_adapter_bind_output_buffers( struct SM64MarioGeometryBuffers *outBuffers );