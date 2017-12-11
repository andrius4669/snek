#pragma once

#include <map>
#include <vector>
#include <memory>
#include "kbsubscriber.h"

struct KeyboardPublisher {
	KeyboardPublisher() {}
	KeyboardPublisher(const KeyboardPublisher &) = delete;
	KeyboardPublisher &operator =(const KeyboardPublisher &) = delete;

	static KeyboardPublisher &getInstance();
	
	typedef KeyboardSubscriber::key_t key_t;
	typedef std::shared_ptr<KeyboardSubscriber> subscriber_t;
	void subscribe(subscriber_t ks,key_t key);
	void unsubscribe(subscriber_t ks,key_t key);
	void update(key_t key);
private:
	typedef std::multimap<key_t,subscriber_t> submap_t;
	submap_t subscribers;
} ;
typedef KeyboardPublisher GKbdControl;
