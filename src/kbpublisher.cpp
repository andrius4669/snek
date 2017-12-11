#include "kbpublisher.h"

void KeyboardPublisher::subscribe(subscriber_t ks,SDL_Scancode key)
{
	subscribers.insert(std::pair<SDL_Scancode,subscriber_t>(key,ks));
}

void KeyboardPublisher::unsubscribe(subscriber_t ks,SDL_Scancode key)
{
	std::pair<submap_t::iterator,submap_t::iterator> r = subscribers.equal_range(key);
	for (;;) {
		if (r.first == r.second)
			break;
		auto oit = r.first;
		++r.first;
		if (oit->second != ks)
			continue;
		subscribers.erase(oit);
	}
}

void KeyboardPublisher::update(SDL_Scancode key)
{
	std::pair<submap_t::iterator,submap_t::iterator> r = subscribers.equal_range(key);
	for (;;) {
		if (r.first == r.second)
			break;
		r.first->second->update(key);
		++r.first;
	}
}
