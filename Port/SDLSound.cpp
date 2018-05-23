#include <SDL.h>
#include "SDLSound.h"

static uint8_t *audioBuffer;
static int audioLength;

static SDL_AudioDeviceID audioDevice;
static SDL_AudioSpec audioSpecs;

void VocAudioCallback(void *userdata, uint8_t *stream, int len)
{
	SDL_memset(stream, 127, len);

	if (audioLength <= 0) return;
	if (len == 0) return;

	if (len > audioLength)
		len = audioLength;
	
	SDL_memcpy(stream, audioBuffer, len);

	audioBuffer += len;
	audioLength -= len;
}

int SDLInitSound()
{
	SDL_AudioSpec desSpecs;
	SDL_memset(&desSpecs, 0, sizeof(desSpecs));
	desSpecs.freq = 10000; // Manche Sounds haben 11111 (Man könnte wohl die Samplerate direkt aus der VOC laden...)
	desSpecs.format = AUDIO_U8;
	desSpecs.channels = 1;
	desSpecs.samples = 256;
	desSpecs.callback = VocAudioCallback;
	audioDevice = SDL_OpenAudioDevice(NULL, 0, &desSpecs, &audioSpecs, 0);
	if (audioDevice == 0) return 0;
	return 1;
}

void SDLCloseSound()
{
	SDL_CloseAudioDevice(audioDevice);
}

void SDLClearSound()
{
	audioLength = 0;
}

int SDLSoundCheck()
{
	if (audioLength <= 0) return 0; // Ton läuft nicht
	else return 1; // Ton läuft
}


int SDLPlaySound(uint8_t *vocbuf)
{
	SDL_LockAudioDevice(audioDevice);
	int smplen, freq;

	if (!vocbuf) return 0;

	vocbuf += *((uint16_t*)(vocbuf + 20)); // Header offset -> 1st Block

	if (*vocbuf != 1) return 0; // Not a new sample data block

	smplen = vocbuf[1] + 256 * vocbuf[2] + 65536 * vocbuf[3] - 6; // get block length-head
	freq = (long)1000000 / (long)(256 - vocbuf[4]);       // get sample rate

	if (vocbuf[5] != 0) return 0; // Sample is packed!

	audioBuffer = vocbuf + 6; // jump block type 1 header
	audioLength = smplen;
	SDL_UnlockAudioDevice(audioDevice);
	SDL_PauseAudioDevice(audioDevice, 0);
	return 1;
}