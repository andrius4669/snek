#pragma once

#include <map>
#include <vector>
#include <memory>
#include "kbsubscriber.h"

struct KeyboardPublisher {
	typedef std::shared_ptr<KeyboardSubscriber> subscriber_t;
	void subscribe(subscriber_t ks,int key);
	void unsubscribe(subscriber_t ks,int key);
	void update(int key);
private:
	typedef std::multimap<int,subscriber_t> submap_t;
	submap_t subscribers;
} ;
