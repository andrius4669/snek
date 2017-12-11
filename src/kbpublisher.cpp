#include "kbpublisher.h"
#include "logger.h"

KeyboardPublisher &KeyboardPublisher::getInstance()
{
	static KeyboardPublisher instance;
	return instance;
}

void KeyboardPublisher::subscribe(subscriber_t ks,key_t key)
{
	auto &l = GLog::getInstance();
	l.logf("kpub: subscribe %d -> %p\n",(int)key,ks.get());
	subscribers.insert(std::pair<key_t,subscriber_t>(key,ks));
}

void KeyboardPublisher::unsubscribe(subscriber_t ks,key_t key)
{
	auto &l = GLog::getInstance();
	l.logf("kpub: unsubscribe %d -> %p\n",(int)key,ks.get());
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

void KeyboardPublisher::update(key_t key)
{
	//auto &l = GLog::getInstance();
	//l.logf("kpub: update %d\n",(int)key);
	//for (auto it = subscribers.begin();it != subscribers.end();++it) {
	//	l.logf("* %d -> %p\n",it->first,it->second.get());
	//}
	std::pair<submap_t::iterator,submap_t::iterator> r = subscribers.equal_range(key);
	for (;;) {
		if (r.first == r.second)
			break;
		//l.logf("-> %p\n",r.first->second.get());
		r.first->second->update(key);
		++r.first;
	}
}
