#include "load_audio_data.h"

#include "decomp/tools/convUtils.h"
#include "decomp/audio/load.h"
#include "decomp/audio/load_dat.h"

bool g_is_audio_initialized = false;

extern void load_audio_banks( const uint8_t *rom ) {
    // FIXME: rom_copy purposfully leaks here
    uint8_t *rom_copy = malloc( 0x800000 );

    memcpy( rom_copy, rom, 0x800000 );

    gSoundDataADSR = parse_seqfile( rom_copy+0x57B720 ); //ctl
    gSoundDataRaw = parse_seqfile( rom_copy+0x593560 ); //tbl
    gMusicData = parse_seqfile( rom_copy+0x7B0860 );
    gBankSetsData = rom_copy+0x7CC621;
    memmove( gBankSetsData+0x45,gBankSetsData+0x45-1,0x5B );
    gBankSetsData[0x45]=0x00;
    ptrs_to_offsets( gSoundDataADSR );

    audio_init();
    sound_init();
    sound_reset( 0 );

    g_is_audio_initialized = true;
}