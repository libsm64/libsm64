#include "load_audio_data.h"

#include "decomp/tools/convUtils.h"
#include "decomp/audio/load.h"
#include "decomp/audio/load_dat.h"

bool is_audio_initialized = false;

extern void load_audio_banks( uint8_t *rom ) {
    uint8_t *rom2 = malloc( 0x800000 );

    memcpy( rom2, rom, 0x800000 );
    rom = rom2;
    gSoundDataADSR = parse_seqfile( rom+0x57B720 ); //ctl
    gSoundDataRaw = parse_seqfile( rom+0x593560 ); //tbl
    gMusicData = parse_seqfile( rom+0x7B0860 );
    gBankSetsData = rom+0x7CC621;
    memmove( gBankSetsData+0x45,gBankSetsData+0x45-1,0x5B );
    gBankSetsData[0x45]=0x00;
    ptrs_to_offsets( gSoundDataADSR );

    audio_init();  
    sound_init();
    sound_reset( 0 );
    
    is_audio_initialized = true;
}