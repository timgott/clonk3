#include <SDL.h>
#include "SDLmain.h"

SDL_Window* SdlWindow;
SDL_Surface* SdlScreenSurface;
int ScreenScaleFactor;

int InitSDL(int scale)
{
	ScreenScaleFactor = scale;
	if (ScreenScaleFactor < 1) ScreenScaleFactor = 1;
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return 0;
	SdlWindow = SDL_CreateWindow(
		"Clonk 3 Radikal", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320 * ScreenScaleFactor, 200 * ScreenScaleFactor, SDL_WINDOW_SHOWN);
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