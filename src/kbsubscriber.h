#pragma once

#include "SDL.h"

struct KeyboardSubscriber {
	typedef SDL_Keycode key_t;
	virtual void update(key_t key) = 0;
} ;
