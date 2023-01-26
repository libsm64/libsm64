#include "audio.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

extern "C" {
#include "../src/libsm64.h"
#include "context.h"
}

static SDL_AudioDeviceID dev;

pthread_t gSoundThread;
long long timeInMilliseconds(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return(((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

void* audio_thread(void* keepAlive)
{
	// from https://github.com/ckosmic/libsm64/blob/audio/src/libsm64.c#L535-L555
	// except keepAlive is a null pointer here, so don't use it

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

	long long currentTime = timeInMilliseconds();
	long long targetTime = 0;
	while(1)
	{
		//if(!*((bool*)keepAlive)) return NULL;

		int16_t audioBuffer[544 * 2 * 2];
		uint32_t numSamples = sm64_audio_tick(SDL_GetQueuedAudioSize(dev)/4, 1100, audioBuffer);
		if (SDL_GetQueuedAudioSize(dev)/4 < 6000)
			SDL_QueueAudio(dev, audioBuffer, numSamples * 2 * 4);

		targetTime = currentTime + 33;
		while (timeInMilliseconds() < targetTime)
		{
			usleep(100);
			//if(!*((bool*)keepAlive)) return NULL;
		}
		currentTime = timeInMilliseconds();
	}
}

void audio_init()
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "SDL_InitSubSystem(SDL_INIT_AUDIO) failed: %s\n", SDL_GetError());
		return;
	}

	SDL_AudioSpec want, have;
	SDL_zero(want);
	want.freq = 32000;
	want.format = AUDIO_S16;
	want.channels = 2;
	want.samples = 512;
	want.callback = NULL;
	dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (dev == 0) {
		fprintf(stderr, "SDL_OpenAudio error: %s\n", SDL_GetError());
		return;
	}
	SDL_PauseAudioDevice(dev, 0);

	// it's best to run audio in a separate thread
	pthread_create(&gSoundThread, NULL, audio_thread, NULL);
}

