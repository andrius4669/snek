#include "kbpublisher.h"

void KeyboardPublisher::subscribe(subscriber_t ks,int key)
{
	subscribers.insert(std::pair<int,subscriber_t>(key,ks));
}

void KeyboardPublisher::unsubscribe(subscriber_t ks,int key)
{
	std::pair<submap_t::iterator,submap_t::iterator> r = subscribers.equal_range(key);
	for (;;) {
		if (r.first == r.second)
			break;
		auto oit = r.first;
		++r.first;
		subscribers.erase(oit);
	}
}

void KeyboardPublisher::update(int key)
{
	std::pair<submap_t::iterator,submap_t::iterator> r = subscribers.equal_range(key);
	for (;;) {
		if (r.first == r.second)
			break;
		r.first->second->update(key);
		++r.first;
	}
}