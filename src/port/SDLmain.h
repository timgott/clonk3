#pragma once

#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include "Scaling.h"

extern SDL_Surface* SdlScreenSurface;
extern SDL_Window* SdlWindow;

extern int PortScreenScaleFactor;
extern UpscaleInterpolationType PortInterpolationType;

int InitSDL(int scale, UpscaleInterpolationType interpolationType);
void CloseSDL(void);
