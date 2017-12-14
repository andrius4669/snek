#pragma once

#include "kbsubscriber.h"
#include "kbpublisher.h"

struct KeyboardMapper {
	KeyboardMapper(
		KeyboardSubscriber::key_t l,
		KeyboardSubscriber::key_t r,
		KeyboardSubscriber::key_t u,
		KeyboardSubscriber::key_t d)
	{
		codes[DIR_LEFT] = l;
		codes[DIR_RIGHT] = r;
		codes[DIR_UP] = u;
		codes[DIR_DOWN] = d;
	}

	void subscribe(KeyboardPublisher &pub,KeyboardPublisher::subscriber_t sub)
	{
		pub.subscribe(sub,codes[DIR_LEFT]);
		pub.subscribe(sub,codes[DIR_RIGHT]);
		pub.subscribe(sub,codes[DIR_UP]);
		pub.subscribe(sub,codes[DIR_DOWN]);
	}
	
	int getDirection(KeyboardSubscriber::key_t scan) const
	{
		for (int i = 0;i < DIR_NUM;++i) {
			if (codes[i] == scan)
				return i;
		}
		return -1;
	}
private:
	KeyboardSubscriber::key_t codes[DIR_NUM];
} ;
