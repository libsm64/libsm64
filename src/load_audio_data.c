#include "load_audio_data.h"

#include "debug_print.h"
#include "decomp/tools/convUtils.h"
#include "decomp/audio/load.h"
#include "decomp/audio/load_dat.h"

extern void load_audio_banks( uint8_t *rom ) {
  DEBUG_PRINT("load_audio_banks()");
  DEBUG_PRINT("- malloc");
  uint8_t *rom2 = malloc(0x800000);

  DEBUG_PRINT("- memcpy");
  memcpy(rom2, rom, 0x800000);
  rom = rom2;
  DEBUG_PRINT("- parse ctl");
  gSoundDataADSR = parse_seqfile(rom+0x57B720); //ctl
  DEBUG_PRINT("- parse tbl");
  gSoundDataRaw = parse_seqfile(rom+0x593560); //tbl
  DEBUG_PRINT("- parse music");
  gMusicData = parse_seqfile(rom+0x7B0860);
  gBankSetsData = rom+0x7CC621;
  DEBUG_PRINT("- memmove");
  memmove(gBankSetsData+0x45,gBankSetsData+0x45-1,0x5B);
  gBankSetsData[0x45]=0x00;
  DEBUG_PRINT("- ptrs to offsets");
  ptrs_to_offsets(gSoundDataADSR);
	
  DEBUG_PRINT("- audio init");
  audio_init();  
  DEBUG_PRINT("- sound init");
  sound_init();
  DEBUG_PRINT("- sound reset");
  sound_reset(0);
  DEBUG_PRINT("- done with audio init!");
}