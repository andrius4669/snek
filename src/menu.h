#pragma once

#include "kbsubscriber.h"
#include "SDL.h"
#include <string>
#include <vector>

struct PlayerConfig {
	size_t keyboardSet;
	std::string playername;
} ;

struct MenuEntry {
	virtual void onEnter() = 0;
	virtual void draw(SDL_Surface *surf,SDL_Rect &rect) = 0;
	
	virtual ~MenuEntry();
} ;

struct StartEntry: MenuEntry {
	StartEntry() = delete;
	StartEntry(const StartEntry &) = delete;
	StartEntry &operator =(const StartEntry &) = delete;
	StartEntry(SDL_Renderer *rend,bool &startptr):
		rend(rend),startptr(&startptr)
	{
		TTF_Font *sans = TTF_OpenFont("Sans.ttf",24);
		SDL_Color white = {255,255,255};
		msgsurf = TTF_RenderText_Solid(
			sans,"BEGIN",white);
		assert(msgsurf != nullptr);
		msgtex =
			SDL_CreateTextureFromSurface(rend,msgsurf);
		assert(msgtex != nullptr);
	}

	~StartEntry()
	{
		SDL_DestroyTexture(msgtex);
		SDL_FreeSurface(msgsurf);
	}

	virtual void onEnter() { *startptr = true; }

	virtual void draw(SDL_Surface *surf,SDL_Rect &rect)
	{
		SDL_RenderCopy(rend,msgtex,NULL,&rect);
	}
private:
	SDL_Renderer *rend;
	bool *startptr;

	SDL_Surface *msgsurf;
	SDL_Texture *msgtex;
} ;

struct PlayerConfigEntry: MenuEntry {
	PlayerConfigEntry() = delete;
	PlayerConfigEntry(SDL_Renderer *rend,PlayerConfig &cfg):
		cfg(&cfg)
	{}

	virtual void onEnter()
	{
		cfg->keyboardSet = (cfg->keyboardSet + 1) % 7;
	}

	virtual void draw(SDL_Surface *surf,SDL_Rect &rect)
	{
		// TODO
	}
private:
	PlayerConfig *cfg;
} ;

struct Menu: KeyboardSubscriber {
	std::vector<PlayerConfig> players;
	
	Menu(SDL_Renderer *rend,size_t width,size_t height):
		width(width),height(height)
	{
		players.push_back({1,""});
		players.push_back({0,""});
		players.push_back({0,""});
		players.push_back({0,""});
		players.push_back({0,""});
		players.push_back({0,""});
		menu.push_back(std::make_shared<StartEntry>(rend,start));
		for (size_t i = 0;i < players.size();++i)
			menu.push_back(
				std::make_shared<PlayerConfigEntry>(
					rend,players[i]));
	}

	virtual void update(KeyboardSubscriber::key_t scan)
	{
		switch (scan) {
			case SDL_SCANCODE_UP:
				
		}
	}

	inline bool needsredraw() const { return updated; }

	void draw(SDL_Surface *surf)
	{
		
		updated = false;
	}

private:
	bool updated = true;
	size_t width,height;
	std::vector<std::shared_ptr<MenuEntry>> menu;
} ;
