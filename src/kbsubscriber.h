#pragma once

#include "SDL.h"

struct KeyboardSubscriber {
	virtual void update(SDL_Scancode keypressed) = 0;
} ;
