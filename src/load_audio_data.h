#pragma once

struct AudioBanks {
  int numCtlEntries;
  struct CtlEntry * ctlEntries;
};

extern struct AudioBanks load_audio_banks();