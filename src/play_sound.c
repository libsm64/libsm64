#include "play_sound.h"

SM64PlaySoundFunctionPtr g_play_sound_func = NULL;

extern void play_sound( uint32_t soundBits, f32 *pos ) {
  if ( g_play_sound_func ) {
    g_play_sound_func(soundBits, pos);
  }
}