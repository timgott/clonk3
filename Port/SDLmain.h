#pragma once

//#include <SDL_video.h>

extern SDL_Surface* SdlScreenSurface;
extern SDL_Window* SdlWindow;
extern int ScreenScaleFactor;

int InitSDL(int scale);
void CloseSDL(void);

//void HandleSDLEvents();


