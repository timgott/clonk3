#include <time.h>
#include <SDL.h>
#include "SDLmain.h"
#include "vga4.h"
#include "UserInput.h"

int WaitForInputTimeout(int maxSeconds)
{
	UpdateScreen();
	SDL_Event e;
	int startTime = time(NULL);
	int leftTime = maxSeconds;
	while (SDL_WaitEventTimeout(&e, leftTime * 1000))
	{
		switch (e.type)
		{
		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_EXPOSED)
				UpdateScreen();
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (e.button.clicks = 1)
				return 0;
			break;
		case SDL_KEYDOWN:
			if (e.key.repeat) break;
			if (e.key.keysym.sym == SDLK_ESCAPE) return 1;
			else return 0;

		case SDL_QUIT:
			return 1;

		default:
			leftTime = (startTime + maxSeconds) - time(NULL);
			break;
		}
	}
}

void WaitForInput()
{
	UpdateScreen();
	SDL_Event e;
	while (SDL_WaitEvent(&e))
	{
		switch (e.type)
		{
		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_EXPOSED)
				UpdateScreen();
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (e.button.clicks = 1)
				return;
			break;
		case SDL_KEYDOWN:
			return;
		case SDL_QUIT:
			return ;
		}
	}
}


bool CheckKeyOrMouse()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_MOUSEBUTTONDOWN:
		case SDL_KEYDOWN:
		case SDL_QUIT:
			return true;
		}
	}
	return false;
}

int GetKey()
{
	int inc, exinc;
	SDL_Event e;

	while (SDL_WaitEvent(&e))
	{
		switch (e.type)
		{
		case SDL_KEYDOWN:
			RemapKey(e.key.keysym.sym, &inc, &exinc);
			if (inc != 0) return inc;
			else return exinc;
		case SDL_QUIT:
			return SDLK_ESCAPE;
		}
	}

	return SDLK_ESCAPE;
}

void RemapKey(SDL_Keycode key, int *inc, int *exinc)
{
	// Remap Keypad.
	if (key >= SDLK_KP_1 && key <= SDLK_KP_9) key -= SDLK_KP_1 - SDLK_1;
	if (key == SDLK_KP_0) key = SDLK_0;

	int lowkey = 0x0000FFFF & key;
	int highkey = 0xFFFF0000 & key;

	*inc = 0;
	*exinc = 0;
	if (highkey == 0) *inc = lowkey;
	else *exinc = lowkey;
}
