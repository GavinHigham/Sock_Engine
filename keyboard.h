#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL2/SDL.h>

extern const Uint8 *key_state;
void init_keyboard();
void keyevent(SDL_Keysym keysym, SDL_EventType type);

#endif