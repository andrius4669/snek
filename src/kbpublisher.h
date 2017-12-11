#pragma once

#include <map>
#include <vector>
#include <memory>
#include "kbsubscriber.h"
#include "singleton.h"

struct KeyboardPublisher {
	typedef std::shared_ptr<KeyboardSubscriber> subscriber_t;
	void subscribe(subscriber_t ks,SDL_Scancode key);
	void unsubscribe(subscriber_t ks,SDL_Scancode key);
	void update(SDL_Scancode key);
private:
	typedef std::multimap<SDL_Scancode,subscriber_t> submap_t;
	submap_t subscribers;
} ;
typedef Singleton<KeyboardPublisher> GKbdControl;
