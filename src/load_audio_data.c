#include "load_audio_data.h"

#include "decomp/audio/load.h"

extern struct AudioBanks load_audio_banks() {
  int numBanks = 10;
  for (int i = 0; i < 10; ++i) {
    bank_load_immediate(i, 2);
  }
  
  struct AudioBanks audioBanks;
  audioBanks.numCtlEntries = numBanks;
  audioBanks.ctlEntries = gCtlEntries;
  
  return audioBanks;
}