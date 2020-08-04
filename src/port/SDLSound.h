#pragma once

#include <stdint.h>

int SDLInitSound();
void SDLCloseSound();
void SDLClearSound();
int SDLSoundCheck();
int SDLPlaySound(uint8_t *vocbuf);
