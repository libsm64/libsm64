#include <stdlib.h>
#include <string.h>

#include "libsm64.h"
#include "engine/math_util.h"
#include "guMtxF2L.h"
#include "gfx_adapter.h"
#include "gfx_adapter_commands.h"

static Mat4 s_curMatrix;
static float s_curColor[3];
static struct SM64MarioGeometryBuffers *s_outBuffers;
static float *s_trianglePtr;
static float *s_colorPtr;
static float *s_normalPtr;

static void mtxf_mul_vec3f(Mat4 mtx, Vec3f b, float w, Vec3f out)
{
    out[0] = b[0] * mtx[0][0] + b[1] * mtx[1][0] + b[2] * mtx[2][0] + w * mtx[3][0];
    out[1] = b[0] * mtx[0][1] + b[1] * mtx[1][1] + b[2] * mtx[2][1] + w * mtx[3][1];
    out[2] = b[0] * mtx[0][2] + b[1] * mtx[1][2] + b[2] * mtx[2][2] + w * mtx[3][2];
}

static void process_display_list( void *dl )
{
    int64_t *ptr = (int64_t *)dl;
    Vtx *vdata = NULL;

    for( ;; )
    {
        switch( *ptr++ )
        {
            case GFXCMD_VertexData: 
            {
                UNUSED int64_t v = *ptr++;
                UNUSED int64_t n = *ptr++;
                UNUSED int64_t v0 = *ptr++;
                vdata = (Vtx*)v;
                break;
            }

            case GFXCMD_Triangle:
            {
                int64_t v00 = *ptr++;
                int64_t v01 = *ptr++;
                int64_t v02 = *ptr++;
                UNUSED int64_t flag0 = *ptr++;

                short x0 = vdata[v00].v.ob[0], y0 = vdata[v00].v.ob[1], z0 = vdata[v00].v.ob[2];
                short x1 = vdata[v01].v.ob[0], y1 = vdata[v01].v.ob[1], z1 = vdata[v01].v.ob[2];
                short x2 = vdata[v02].v.ob[0], y2 = vdata[v02].v.ob[1], z2 = vdata[v02].v.ob[2];
                Vec3f p0 = { (float)x0, (float)y0, (float)z0 };
                Vec3f p1 = { (float)x1, (float)y1, (float)z1 };
                Vec3f p2 = { (float)x2, (float)y2, (float)z2 };

                signed char nx0 = vdata[v00].n.n[0], ny0 = vdata[v00].n.n[1], nz0 = vdata[v00].n.n[2];
                signed char nx1 = vdata[v01].n.n[0], ny1 = vdata[v01].n.n[1], nz1 = vdata[v01].n.n[2];
                signed char nx2 = vdata[v02].n.n[0], ny2 = vdata[v02].n.n[1], nz2 = vdata[v02].n.n[2];
                Vec3f n0 = { ((float)nx0) / 128.0f, ((float)ny0) / 128.0f, ((float)nz0) / 128.0f };
                Vec3f n1 = { ((float)nx1) / 128.0f, ((float)ny1) / 128.0f, ((float)nz1) / 128.0f };
                Vec3f n2 = { ((float)nx2) / 128.0f, ((float)ny2) / 128.0f, ((float)nz2) / 128.0f };

                mtxf_mul_vec3f( s_curMatrix, p0, 1.0f, s_trianglePtr );
                s_trianglePtr += 3;
                mtxf_mul_vec3f( s_curMatrix, p1, 1.0f, s_trianglePtr );
                s_trianglePtr += 3;
                mtxf_mul_vec3f( s_curMatrix, p2, 1.0f, s_trianglePtr );
                s_trianglePtr += 3;

                // TODO normals arent correct under non-uniform scale. multiple by inverse/transpose
                mtxf_mul_vec3f( s_curMatrix, n0, 0.0f, s_normalPtr );
                vec3f_normalize( s_normalPtr );
                s_normalPtr += 3;
                mtxf_mul_vec3f( s_curMatrix, n1, 0.0f, s_normalPtr );
                vec3f_normalize( s_normalPtr );
                s_normalPtr += 3;
                mtxf_mul_vec3f( s_curMatrix, n2, 0.0f, s_normalPtr );
                vec3f_normalize( s_normalPtr );
                s_normalPtr += 3;

                *s_colorPtr++ = s_curColor[0];
                *s_colorPtr++ = s_curColor[1];
                *s_colorPtr++ = s_curColor[2];
                *s_colorPtr++ = s_curColor[0];
                *s_colorPtr++ = s_curColor[1];
                *s_colorPtr++ = s_curColor[2];
                *s_colorPtr++ = s_curColor[0];
                *s_colorPtr++ = s_curColor[1];
                *s_colorPtr++ = s_curColor[2];

                break;
            }

            case GFXCMD_Light:
            {
                int64_t l = *ptr++;
                int64_t n = *ptr++;

                if( n == 1 )
                {
                    Light *data = (Light*)l;
                    s_curColor[0] = (float)data->l.col[0] / 255.0f;
                    s_curColor[1] = (float)data->l.col[1] / 255.0f;
                    s_curColor[2] = (float)data->l.col[2] / 255.0f;
                }
                
                break;
            }

            case GFXCMD_SubDisplayList:
            {
                int64_t dl = *ptr++;
                process_display_list( (void*)dl );
                break;
            }

            case GFXCMD_EndDisplayList:
                goto break_top;
        }
    }

    s_outBuffers->bufferUsedSize = s_trianglePtr - s_outBuffers->position;

break_top:
    {}
}

void gSPMatrix( void *pkt, Mtx *m, uint8_t flags )
{
    guMtxL2F( s_curMatrix, m );
}

void gSPDisplayList( void *pkt, struct DisplayListNode *dl )
{
    process_display_list( (void*)dl );
}

void gfx_adapter_bind_output_buffers( struct SM64MarioGeometryBuffers *outBuffers )
{
    s_outBuffers = outBuffers;
    s_trianglePtr = s_outBuffers->position;
    s_colorPtr = s_outBuffers->color;
    s_normalPtr = s_outBuffers->normal;
    s_outBuffers->bufferUsedSize = 0;
}