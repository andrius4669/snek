#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include "logger.h"

int main(int argc,char **argv)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr,"SDL_Init failed\n");
		exit(1);
	}
	atexit(SDL_Quit);

	auto &l = GLog::getInstance();
	l.log("hello world\n");
	
	return 0;
}
