#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL.h>
#include "logger.h"
#include "kbsubscriber.h"
#include "kbpublisher.h"
#include "palette.h"

#include <memory>
#include <vector>
#include <queue>

#include "fieldobject.h"
#include "field.h"
#include "playerhead.h"
#include "gamemaster.h"

#define FIELDX 128
#define FIELDY 80
#define CELLXSIZE 8
#define CELLYSIZE 8

struct QuitNotifier: KeyboardSubscriber {
	virtual void update(KeyboardSubscriber::key_t scan)
	{
		quit = true;
	}

	void set()
	{
		quit = true;
	}
	
	bool isset() const { return quit; }
private:
	bool quit = false;
} ;

static std::vector<std::shared_ptr<FieldObject>> objs;

static void advance()
{
	for (size_t i = 0;i < objs.size();++i) {
		objs[i]->advance();
	}
}

struct PaletteVal {
	unsigned char r,g,b;
} ;

static std::vector<PaletteVal> Palette;
static std::vector<int> SnakePalette;

static void initPalette()
{
	// TODO read from config
	
	Palette.push_back({15,15,0});
	Palette.push_back({240,255,255});
	
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({255,0,0});
	Palette.push_back({127,0,0});
	Palette.push_back({200,0,0});
	Palette.push_back({100,0,0});
	
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({0,255,0});
	Palette.push_back({0,127,0});
	Palette.push_back({0,200,0});
	Palette.push_back({0,100,0});
	
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({0,0,255});
	Palette.push_back({0,0,127});
	Palette.push_back({0,0,200});
	Palette.push_back({0,0,100});
	
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({255,255,0});
	Palette.push_back({127,127,0});
	Palette.push_back({200,200,0});
	Palette.push_back({100,100,0});
	
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({255,0,255});
	Palette.push_back({127,0,127});
	Palette.push_back({200,0,200});
	Palette.push_back({100,0,100});
	
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({0,255,255});
	Palette.push_back({0,127,127});
	Palette.push_back({0,200,200});
	Palette.push_back({0,100,100});
	
	Palette.push_back({0,255,63});
	Palette.push_back({255,63,0});
}

static void drawField(SDL_Surface *surf,const std::shared_ptr<Field> &field)
{
	SDL_Rect rect;
	memset(&rect,0,sizeof(rect));
	rect.w = CELLXSIZE;
	rect.h = CELLYSIZE;
	for (size_t y = 0;y < field->fheight;++y) {
		for (size_t x = 0;x < field->fwidth;++x) {
			auto t = field->access(x,y)->getTexture();
			assert(t >= 0 && t < Palette.size());
			auto &pv = Palette[t];
			rect.x = x * CELLXSIZE;
			rect.y = y * CELLYSIZE;
			int r = SDL_FillRect(surf,&rect,SDL_MapRGB(surf->format,pv.r,pv.g,pv.b));
			assert(r == 0);
		}
	}
}

static void startgame(std::shared_ptr<Field> &field)
{
	auto &l = GLog::getInstance();
	l.log("on startgame\n");
	assert(SnakePalette.size() >= 1);
	KeyboardMapper map(
		SDL_SCANCODE_A,
		SDL_SCANCODE_D,
		SDL_SCANCODE_W,
		SDL_SCANCODE_S); // WASD
	auto snek = std::make_shared<PlayerHead>(SnakePalette[0],5,5,field,map);
	snek->initSelf(snek);
	objs.push_back(snek);
	
	auto gm = std::make_shared<GameMaster>(field);
	objs.push_back(gm);
	
	l.log("after startgame\n");
}

static void redraw(SDL_Window *w,std::shared_ptr<Field> &field,bool force)
{
	if (force || field->isChanged()) {
		SDL_Surface *surf = SDL_GetWindowSurface(w); 
		assert(surf != nullptr);

		drawField(surf,field);
		field->clearChanged();

		SDL_UpdateWindowSurface(w);
	}
}

int main(int argc,char **argv)
{
	auto &l = GLog::getInstance();
	
	l.log("hello world\n");
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		l.logf("SDL_Init failed: %s\n",SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	std::shared_ptr<Field> field = std::make_shared<Field>(FIELDX,FIELDY);
	SDL_Window *w = SDL_CreateWindow("SNEK",
		SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
		field->fwidth * CELLXSIZE,field->fheight * CELLYSIZE,SDL_WINDOW_SHOWN);
	if (!w) {
		l.logf("failed to create window: %s\n",SDL_GetError());
		return 1;
	}

	auto wid = SDL_GetWindowID(w);

	auto &k = GKbdControl::getInstance();
	auto qn = std::make_shared<QuitNotifier>();
	k.subscribe(qn,SDL_SCANCODE_ESCAPE);

	initPalette();
	startgame(field);

	bool needredraw = true;

	unsigned int nextadvance = SDL_GetTicks();

	for (;;) {
		SDL_Event ev;
		int st = 0;
		int r;
		// we first use PollEvent
		// then if we find something, continue using PollEvent till we get nothing
		// then if we find nothing, use WaitEventTimeout once
		for (;;) {
			if (st == 0) {
				r = SDL_PollEvent(&ev);
				if (!r) {
					r = SDL_WaitEventTimeout(&ev,5);
					if (!r)
						break;
					st = 1;
				}
				else {
					st = 2;
				}
			}
			else if (st == 1) {
				break;
			}
			else if (st == 2) {
				r = SDL_PollEvent(&ev);
				if (!r)
					break;
			}
			else
				assert(0);

			switch (ev.type) {
				case SDL_QUIT:
					//l.log("quit event received\n");
					qn->set();
					break;
				case SDL_KEYDOWN: {
					//l.log("keydown event received\n");
					k.update(ev.key.keysym.scancode);
					break;
				}
				case SDL_WINDOWEVENT:
					if (ev.window.windowID == wid) {
						switch (ev.window.event) {
							case SDL_WINDOWEVENT_EXPOSED:
							case SDL_WINDOWEVENT_SIZE_CHANGED:
							case SDL_WINDOWEVENT_SHOWN:
								needredraw = true;
								break;
						}
					}
			}
		}

		unsigned int t = SDL_GetTicks();
		while ((int)(t - nextadvance) >= 0) {
			advance();
			nextadvance += 100;
		}

		redraw(w,field,needredraw);

		if (qn->isset())
			break;
	}
	
	SDL_DestroyWindow(w);
	
	return 0;
}
