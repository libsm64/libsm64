#include "load_anim_data.h"

#include <stdlib.h>

static uint32_t s_num_entries = 0;
static struct Animation *s_libsm64_mario_animations = NULL;

#define ANIM_DATA_ADDRESS 0x004EC000

static uint16_t read_u16_be( const uint8_t *p )
{
    return
        (uint32_t)p[0] << 8 |
        (uint32_t)p[1];

}

static uint16_t read_s16_be( const uint8_t *p )
{
    return (int16_t)read_u16_be( p );
}

static uint32_t read_u32_be( const uint8_t *p )
{
    return
        (uint32_t)p[0] << 24 |
        (uint32_t)p[1] << 16 |
        (uint32_t)p[2] <<  8 |
        (uint32_t)p[3];
}

void load_mario_anims_from_rom( const uint8_t *rom )
{
    #define GET_OFFSET( n ) (read_u32_be((uint8_t*)&((struct OffsetSizePair*)( rom + ANIM_DATA_ADDRESS + 8 + (n)*8 ))->offset))
    #define GET_SIZE(   n ) (read_u32_be((uint8_t*)&((struct OffsetSizePair*)( rom + ANIM_DATA_ADDRESS + 8 + (n)*8 ))->size  ))

    const uint8_t *read_ptr = rom + ANIM_DATA_ADDRESS;
    s_num_entries = read_u32_be( read_ptr );

    s_libsm64_mario_animations = malloc( s_num_entries * sizeof( struct Animation ));
    struct Animation *anims = s_libsm64_mario_animations;

    for( int i = 0; i < s_num_entries; ++i )
    {
        read_ptr = rom + ANIM_DATA_ADDRESS + GET_OFFSET(i);

        anims[i].flags             = read_s16_be( read_ptr ); read_ptr += 2;
        anims[i].animYTransDivisor = read_s16_be( read_ptr ); read_ptr += 2;
        anims[i].startFrame        = read_s16_be( read_ptr ); read_ptr += 2;
        anims[i].loopStart         = read_s16_be( read_ptr ); read_ptr += 2;
        anims[i].loopEnd           = read_s16_be( read_ptr ); read_ptr += 2;
        anims[i].unusedBoneCount   = read_s16_be( read_ptr ); read_ptr += 2;
        uint32_t values_offset = read_u32_be( read_ptr ); read_ptr += 4;
        uint32_t index_offset  = read_u32_be( read_ptr ); read_ptr += 4;
        uint32_t end_offset    = read_u32_be( read_ptr );

        read_ptr                  = rom + ANIM_DATA_ADDRESS + GET_OFFSET(i) + index_offset;
        const uint8_t *values_ptr = rom + ANIM_DATA_ADDRESS + GET_OFFSET(i) + values_offset;
        const uint8_t *end_ptr    = rom + ANIM_DATA_ADDRESS + GET_OFFSET(i) + end_offset;

        anims[i].index = malloc( values_offset - index_offset );
        anims[i].values = malloc( end_offset - values_offset );

        int j = 0;
        while( read_ptr < values_ptr )
        {
            anims[i].index[j++] = read_u16_be( read_ptr );
            read_ptr += 2;
        }

        j = 0;
        while( read_ptr < end_ptr )
        {
            anims[i].values[j++] = read_u16_be( read_ptr );
            read_ptr += 2;
        }
    }

    #undef GET_OFFSET
    #undef GET_SIZE
}

void load_mario_animation(struct MarioAnimation *a, u32 index)
{
    if (a->currentAnimAddr != 1 + index) {
        a->currentAnimAddr = 1 + index;
        a->targetAnim = &s_libsm64_mario_animations[index];
    }
}

void unload_mario_anims( void )
{
    for( int i = 0; i < s_num_entries; ++i )
    {
        free( s_libsm64_mario_animations[i].index );
        free( s_libsm64_mario_animations[i].values );
    }

    free( s_libsm64_mario_animations );
    s_libsm64_mario_animations = NULL;
    s_num_entries = 0;
}