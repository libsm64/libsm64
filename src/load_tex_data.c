#include "load_tex_data.h"

#include <stddef.h>
#include <string.h>

#include "tools/libmio0.h"
#include "tools/n64graphics.h"

uint8_t *gLibSm64TextureRgba;

#define MARIO_TEX_ROM_OFFSET 1132368
#define NUM_USED_TEXTURES 11
#define ATLAS_WIDTH (NUM_USED_TEXTURES * 64)
#define ATLAS_HEIGHT 64

static int mario_tex_offsets[NUM_USED_TEXTURES] = { 144, 4240, 6288, 8336, 10384, 12432, 14480, 16528, 30864, 32912, 37008 };
static int mario_tex_widths [NUM_USED_TEXTURES] = { 64, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 };
static int mario_tex_heights[NUM_USED_TEXTURES] = { 32, 32, 32, 32, 32, 32, 32, 32, 32, 64, 64 };

static void blt_image_to_atlas( rgba *img, int i, int w, int h )
{
    for( int iy = 0; iy < h; ++iy )
    for( int ix = 0; ix < w; ++ix )
    {
        int o = (ix + 64 * i) + iy * ATLAS_WIDTH;
        int q = ix + iy * w;
        gLibSm64TextureRgba[4*o + 0] = img[q].red;
        gLibSm64TextureRgba[4*o + 1] = img[q].green;
        gLibSm64TextureRgba[4*o + 2] = img[q].blue;
        gLibSm64TextureRgba[4*o + 3] = img[q].alpha;
    }
}

void load_mario_textures_from_rom( uint8_t *rom )
{
    gLibSm64TextureRgba = malloc( 4 * ATLAS_WIDTH * ATLAS_HEIGHT );
    memset( gLibSm64TextureRgba, 0, 4 * ATLAS_WIDTH * ATLAS_HEIGHT );

    mio0_header_t head;
    uint8_t *in_buf = rom + MARIO_TEX_ROM_OFFSET;

    mio0_decode_header( in_buf, &head );
    uint8_t *out_buf = malloc( head.dest_size );
    int bytes_read = mio0_decode( in_buf, out_buf, NULL );

    for( int i = 0; i < NUM_USED_TEXTURES; ++i )
    {
        uint8_t *raw = out_buf + mario_tex_offsets[i];
        rgba *img = raw2rgba( raw, mario_tex_widths[i], mario_tex_heights[i], 16 );
        blt_image_to_atlas( img, i, mario_tex_widths[i], mario_tex_heights[i] );
        free( img );
    }

    free( out_buf );
}