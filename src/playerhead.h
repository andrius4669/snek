#pragma once

#include "playertail.h"
#include "itemobject.h"
#include "keyboardmapper.h"

struct PlayerHead: FieldObject,KeyboardSubscriber {
	virtual size_t getTexture() const { return texture; }
	virtual int getHardness() const { return 1; }
	virtual bool isDead() const { return dead; }
	virtual bool isActive() const { return !dead; }

	virtual void update(KeyboardSubscriber::key_t scan)
	{
		int dir = kbmapper.getDirection(scan);
		assert(dir >= 0);
		//auto &l = GLog::getInstance();
		//l.logf("snake received direction %d\n",dir);
		int lastdir = directions.empty() ? currdir : directions.back();

		if (lastdir == dir)
			return;

		switch (lastdir) {
			case DIR_LEFT:
				if (dir == DIR_RIGHT)
					return;
				break;
			case DIR_RIGHT:
				if (dir == DIR_LEFT)
					return;
				break;
			case DIR_UP:
				if (dir == DIR_DOWN)
					return;
				break;
			case DIR_DOWN:
				if (dir == DIR_UP)
					return;
				break;
		}
		if (dead)
			return;
		directions.push(dir);
	}

	virtual bool canAdvance() const { return !dead; }
	virtual void advance()
	{
		if (!directions.empty()) {
			currdir = directions.front();
			directions.pop();
		}
		if (currdir < 0 || dead)
			return;

		//auto &l = GLog::getInstance();
		//l.logf("advance with direction %d\n",dir);

		ssize_t newx = (ssize_t)x,newy = (ssize_t)y;
		assert(!!field);
		switch (currdir) {
			case DIR_LEFT:
				newx = x - 1;
				if (newx < 0)
					newx = field->fwidth - 1;
				break;
			case DIR_RIGHT:
				newx = (x + 1) % field->fwidth;
				break;
			case DIR_UP:
				newy = y - 1;
				if (newy < 0)
					newy = field->fheight - 1;
				break;
			case DIR_DOWN:
				newy = (y + 1) % field->fheight;
				break;
			default:
				assert(0);
		}
		// check for crash
		assert(!!field->access(newx,newy));
		if (field->access(newx,newy)->getHardness() >= getHardness()) {
			doCrash();
			return;
		}
		// pad tail
		assert(!!tail);
		field->access(x,y) = tail;
		// check item
		auto itm = dynamic_cast<ItemObject *>(field->access(newx,newy).get());
		if (itm)
			itm->onStep(self);
		// change our location
		x = newx;
		y = newy;
		assert(!!self);
		field->access(x,y) = self;
		// we modified field
		field->setChanged();
	}
	
	PlayerHead(int txt, size_t x, size_t y, std::shared_ptr<Field> field,KeyboardMapper mapper):
		texture(txt),tail(std::make_shared<PlayerTail>(txt + 2)),
		x(x),y(y),field(field),dead(false),kbmapper(mapper)
	{}

	void initSelf(std::shared_ptr<PlayerHead> s)
	{
		assert(this == s.get());
		self = s;
		field->access(x,y) = s;
		auto &k = GKbdControl::getInstance();
		kbmapper.subscribe(k,s);
	}

	std::shared_ptr<PlayerTail> getTail() { return tail; }

private:
	int texture;
	std::shared_ptr<PlayerTail> tail;
	std::queue<int> directions;
	int currdir = -1;
	size_t x,y;
	std::shared_ptr<Field> field;
	bool dead;
	std::shared_ptr<PlayerHead> self;
	KeyboardMapper kbmapper;

	void doCrash()
	{
		assert(!dead);
		// color change needs notification too
		field->setChanged();
		dead = true;
		++texture;
		auto *t = dynamic_cast<PlayerTail *>(tail.get());
		assert(t != nullptr);
		t->died();
	}
} ;
