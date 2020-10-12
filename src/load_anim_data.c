#include "load_anim_data.h"
#include "include/types.h"

#include <stdlib.h>

struct Animation *gLibSm64MarioAnimations;
static struct Animation *s_marioAnimations;

#define ANIM_DATA_ADDRESS 0x004EC000

static uint16_t read_u16_be( uint8_t *p )
{
    return 
        (uint32_t)p[0] << 8 |
        (uint32_t)p[1];

}

static uint16_t read_s16_be( uint8_t *p )
{
    return (int16_t)read_u16_be( p );
}

static uint32_t read_u32_be( uint8_t *p )
{
    return
        (uint32_t)p[0] << 24 |
        (uint32_t)p[1] << 16 |
        (uint32_t)p[2] <<  8 |
        (uint32_t)p[3];
}

void load_mario_anims_from_rom( uint8_t *rom )
{
    #define GET_OFFSET( n ) (read_u32_be(&((struct OffsetSizePair*)( rom + ANIM_DATA_ADDRESS + 8 + (n)*8 ))->offset))
    #define GET_SIZE(   n ) (read_u32_be(&((struct OffsetSizePair*)( rom + ANIM_DATA_ADDRESS + 8 + (n)*8 ))->size  ))

    uint8_t *read_ptr = rom + ANIM_DATA_ADDRESS;
    uint32_t num_entries = read_u32_be( read_ptr );

    s_marioAnimations = malloc( num_entries * sizeof( struct Animation ));

    for( int i = 0; i < num_entries; ++i )
    {
        read_ptr = rom + ANIM_DATA_ADDRESS + GET_OFFSET(i);

        s_marioAnimations[i].flags             = read_s16_be( read_ptr ); read_ptr += 2;
        s_marioAnimations[i].animYTransDivisor = read_s16_be( read_ptr ); read_ptr += 2;
        s_marioAnimations[i].startFrame        = read_s16_be( read_ptr ); read_ptr += 2;
        s_marioAnimations[i].loopStart         = read_s16_be( read_ptr ); read_ptr += 2;
        s_marioAnimations[i].loopEnd           = read_s16_be( read_ptr ); read_ptr += 2;
        s_marioAnimations[i].unusedBoneCount   = read_s16_be( read_ptr ); read_ptr += 2;
        uint32_t values_offset = read_u32_be( read_ptr ); read_ptr += 4;
        uint32_t index_offset  = read_u32_be( read_ptr ); read_ptr += 4;
        uint32_t end_offset    = read_u32_be( read_ptr );

        read_ptr            = rom + ANIM_DATA_ADDRESS + GET_OFFSET(i) + index_offset;
        uint8_t *values_ptr = rom + ANIM_DATA_ADDRESS + GET_OFFSET(i) + values_offset;
        uint8_t *end_ptr    = rom + ANIM_DATA_ADDRESS + GET_OFFSET(i) + end_offset;

        s_marioAnimations[i].index = malloc( values_offset - index_offset );
        s_marioAnimations[i].values = malloc( end_offset - values_offset );

        int j = 0;
        while( read_ptr < values_ptr )
        {
            s_marioAnimations[i].index[j++] = read_u16_be( read_ptr );
            read_ptr += 2;
        }

        j = 0;
        while( read_ptr < end_ptr )
        {
            s_marioAnimations[i].values[j++] = read_u16_be( read_ptr );
            read_ptr += 2;
        }
    }

    gLibSm64MarioAnimations = s_marioAnimations;

    #undef GET_OFFSET
    #undef GET_SIZE
}