#pragma once

#include "kbsubscriber.h"
#include "SDL.h"
#include <string>
#include <vector>

struct PlayerConfig {
	size_t keyboardSet;
	std::string playername;
} ;

struct Menu: KeyboardSubscriber {
	std::vector<PlayerConfig> players;
	
	Menu()
	{
		players.push_back({1,""});
		players.push_back({0,""});
		players.push_back({0,""});
		players.push_back({0,""});
		players.push_back({0,""});
		players.push_back({0,""});
	}

	virtual void update(KeyboardSubscriber::key_t scan)
	{
		switch (scan) {
			
		}
	}

	void set()
	{
		quit = true;
	}
	
	bool isset() const { return quit; }
private:
	bool quit = false;
} ;
