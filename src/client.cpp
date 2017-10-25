#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include "logger.h"

int main(int argc,char **argv)
{
	auto &l = GLog::getInstance();
	
	l.log("hello world\n");
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		l.logf("SDL_Init failed: %s",SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	
	auto w = SDL_CreateWindow("SNEK",
		SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
		640,480,SDL_WINDOW_SHOWN);
	if (!w) {
		l.logf("failed to create window: %s",SDL_GetError());
		return 1;
	}
	
	//auto surf = SDL_GetWindowSurface(w); 
	
	
	SDL_Delay(3000);
	SDL_DestroyWindow(w);
	
	return 0;
}
