#include "load_anim_data.h"
#include "include/types.h"
#include "anim_datac.h"

#include <stdlib.h>

static struct MarioAnimsObj s_marioAnimsObj;

#define ANIM_DATA_ADDRESS 0x004EC000

#define MAX_ANIM_DATA_SIZE ( 1024 * 1024 )

void *gMarioAnimsPtr;

uint16_t read_u16_be( uint8_t **ptr )
{
    uint8_t *p = *ptr;

    uint32_t result = 
        (uint32_t)p[0] << 8 |
        (uint32_t)p[1];

    *ptr += 2;

    return result;
}

int16_t read_s16_be( uint8_t **ptr )
{
    return (int16_t)read_u16_be(ptr);
}

uint32_t read_u32_be( uint8_t **ptr )
{
    uint8_t *p = *ptr;

    uint32_t result = 
        (uint32_t)p[0] << 24 |
        (uint32_t)p[1] << 16 |
        (uint32_t)p[2] <<  8 |
        (uint32_t)p[3];

    *ptr += 4;

    return result;
}

void write_u16_se( uint8_t **ptr, uint16_t val )
{
    while( (size_t)(*ptr) % 2 > 0 )
        *ptr += 1;

    uint16_t *p = *ptr;
    *p = val;
    *ptr += 2;
}

void write_s16_se( uint8_t **ptr, int16_t val )
{
    while( (size_t)(*ptr) % 2 > 0 )
        *ptr += 1;

    uint16_t *p = *ptr;
    *p = (uint16_t)val;
    *ptr += 2;
}

void write_u32_se( uint8_t **ptr, uint32_t val )
{
    while( (size_t)(*ptr) % 4 > 0 )
        *ptr += 1;

    uint32_t *p = *ptr;
    *p = val;
    *ptr += 4;
}

void write_u64_se( uint8_t **ptr, uint64_t val )
{
    while( (size_t)(*ptr) % 8 > 0 )
        *ptr += 1;

    uint64_t *p = *ptr;
    *p = val;
    *ptr += 8;
}

#define GET_OFFSET_SIZE_PAIR_PTR( n ) ((struct OffsetSizePair*)( buffer + 8 + sizeof(void*) + (n)*8 ))
#define OVERWRITE_OFFSET( n, v ) do { ((struct OffsetSizePair*)( buffer + 8 + sizeof(void*) + (n)*8 ))->offset = (v); } while(0)
#define OVERWRITE_SIZE( n, v )   do { ((struct OffsetSizePair*)( buffer + 8 + sizeof(void*) + (n)*8 ))->size   = (v); } while(0)




void load_mario_anims_from_rom( uint8_t *rom )
{
    uint8_t *buffer = malloc( MAX_ANIM_DATA_SIZE );
    uint8_t *read_ptr = rom + ANIM_DATA_ADDRESS;
    uint8_t *write_ptr = buffer;

    uint32_t num_entries = read_u32_be( &read_ptr );
    write_u32_se( &write_ptr, num_entries );
    write_u32_se( &write_ptr, 0 );

    read_u32_be( &read_ptr );
    write_u64_se( &write_ptr, 0 );

    // Read all of the offset/size pairs for animation entries from the ROM.
    for( int i = 0; i < num_entries; ++i )
    {
        uint32_t offset = read_u32_be( &read_ptr );
        uint32_t size = read_u32_be( &read_ptr );

        write_u32_se( &write_ptr, offset );
        write_u32_se( &write_ptr, size );
    }

    int ptr_size_diff = sizeof(void*) - 4;

    printf("\n\n");

    for( int i = 0; i < num_entries; ++i )
    {
        struct OffsetSizePair rom_osp = *GET_OFFSET_SIZE_PAIR_PTR(i);
        read_ptr = rom + ANIM_DATA_ADDRESS + rom_osp.offset;

        //offset = offset(top)
        //size = offset(values) + size(values) - offset(top)
        size_t offset = write_ptr - buffer;
        OVERWRITE_OFFSET(i, offset);

        write_s16_se( &write_ptr, read_s16_be( &read_ptr )); // flags
        write_s16_se( &write_ptr, read_s16_be( &read_ptr )); // animYTransDivisor
        write_s16_se( &write_ptr, read_s16_be( &read_ptr )); // startFrame
        write_s16_se( &write_ptr, read_s16_be( &read_ptr )); // loopStart
        write_s16_se( &write_ptr, read_s16_be( &read_ptr )); // loopEnd
        write_s16_se( &write_ptr, read_s16_be( &read_ptr )); // unusedBoneCount

        //values_ptr = offset(values) - offset(top)
        //index_ptr = offset(indices) - offset(top)
        //length = offset(values) + size(values) - offset(top)
         
        uint32_t values_ptr = read_u32_be( &read_ptr ) + 2 * ptr_size_diff;
        uint32_t index_ptr = read_u32_be( &read_ptr ) + 2 * ptr_size_diff;
        uint32_t length = read_u32_be( &read_ptr ) + 4 * ptr_size_diff;

        OVERWRITE_SIZE(i, length);

        printf(" > %x %x \n", offset, length);
        printf(" >>> %x %x \n", gMarioAnims.entries[i].offset, gMarioAnims.entries[i].size);

        write_u64_se( &write_ptr, values_ptr );
        write_u64_se( &write_ptr, index_ptr );
        write_u32_se( &write_ptr, length );

        size_t data_bytes = i < num_entries - 1
            ? rom + ANIM_DATA_ADDRESS + GET_OFFSET_SIZE_PAIR_PTR(i + 1)->offset - read_ptr
            : length;

        for( int i = 0; i < data_bytes; ++i )
            *write_ptr++ = *read_ptr++;

        write_ptr += 8;
    }

    gMarioAnimsPtr = buffer; // &gMarioAnims;
}