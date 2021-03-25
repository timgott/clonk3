#include <SDL.h>
#include "SDLmain.h"
#include "Scaling.h"

SDL_Window* SdlWindow;
SDL_Surface* SdlScreenSurface;
int PortScreenScaleFactor;
UpscaleInterpolationType PortInterpolationType;

int InitSDL(int scale, UpscaleInterpolationType interpolationType)
{
	int minScale = GetMinScaleFactor(interpolationType);
	if (scale < minScale) scale = minScale;
	PortScreenScaleFactor = scale;
	PortInterpolationType = interpolationType;
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return 0;
	SdlWindow = SDL_CreateWindow(
		"Clonk 3 Radikal", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320 * PortScreenScaleFactor, 200 * PortScreenScaleFactor, SDL_WINDOW_SHOWN);
	if (SdlWindow == NULL) return 0;

	SdlScreenSurface = SDL_GetWindowSurface(SdlWindow);

	return 1;
}

void CloseSDL(void)
{
	SDL_DestroyWindow(SdlWindow);
	SdlWindow = NULL;

	SDL_Quit();
}