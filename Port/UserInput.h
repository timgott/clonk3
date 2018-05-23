#pragma once

#include <SDL_keyboard.h>

int WaitForInputTimeout(int maxSeconds);

void WaitForInput();

bool CheckKeyOrMouse();

int GetKey();

void RemapKey(SDL_Keycode key, int * inc, int * exinc);
