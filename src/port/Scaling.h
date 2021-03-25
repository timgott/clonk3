#pragma once

#include <SDL2/SDL_pixels.h>

enum UpscaleInterpolationType {
    INTERPOLATION_NONE,
    INTERPOLATION_SCALE2X,
    INTERPOLATION_SCALE4X
};

void BlitIndexedPixelsAndScale(int w, int h, Uint8 *sourcePixels, Uint32 *destPixels, SDL_Color *palette, int scaleFactor, UpscaleInterpolationType interpolationType);
int GetMinScaleFactor(UpscaleInterpolationType interpolationType);