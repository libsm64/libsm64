#pragma once

#include "decomp/engine/graph_node.h"
#include "libsm64.h"

// Commented out in gbi.h - Replaced here 
#define gDPPipeSync(pkt) ({})
#define gSPSetGeometryMode(pkt, word) ({})
#define gSPClearGeometryMode(pkt, word) ({})
#define gDPFillRectangle(pkt, ulx, uly, lrx, lry) ({})
#define gSPViewport(pkt, v) ({})
extern void gSPMatrix( void *pkt, Mtx *m, uint8_t flags );
extern void gSPDisplayList( void *pkt, struct DisplayListNode *dl );

extern void gfx_adapter_bind_output_buffers( struct SM64MarioGeometryBuffers *outBuffers );