#pragma once

#include <stdint.h>

struct AudioBanks {
  int numCtlEntries;
  struct CtlEntry * ctlEntries;
};

extern struct AudioBanks load_audio_banks( uint8_t *rom );