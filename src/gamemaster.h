#pragma once

#include "fieldobject.h"
#include "aciditem.h"
#include "bombitem.h"
#include <assert.h>

struct GameMaster: FieldObject {
	GameMaster(std::shared_ptr<Field> field):
		field(field)
	{}
	
	virtual size_t getTexture() const { return 0; }
	virtual int getHardness() const { return -1; }
	virtual bool isDead() const { return true; }
	
	virtual bool isActive() const
	{
		for (size_t i = 0;i < objs.size();++i) {
			if (objs[i]->isActive())
				return true;
		}
		return false;
	}
	
	virtual bool canAdvance() const
	{
		return true;
	}
	
	virtual void advance()
	{
		// spawn stuff regulary
		spawnstuff();
		// advance and reap children
		for (size_t i = 0;i < objs.size();++i) {
			objs[i]->advance();
			if (objs[i]->isDead()) {
				objs.erase(objs.begin() + i);
				--i;
			}
		}
	}
private:
	std::shared_ptr<Field> field;
	std::vector<std::shared_ptr<FieldObject>> objs;
	size_t spawnsteps = 0;

	void spawnstuff()
	{
		if (spawnsteps >= 15) {
			spawnsteps -= 10;
			spawnsomething();
		}
		++spawnsteps;
	}
	
	void spawnsomething()
	{
		// uhhhhhh dunno
		for (size_t i = 0;i < 500;++i) {
			size_t x = size_t(rand() % field->fwidth);
			size_t y = size_t(rand() % field->fheight);
			if (field->access(x,y)->getHardness() >= 0)
				continue;
			// ok put it there
			auto itm = randomitem(x,y);
			field->access(x,y) = itm;
			objs.push_back(itm);
			break;
		}
	}
	
	std::shared_ptr<ItemObject> randomitem(size_t x,size_t y)
	{
		int r = rand() % 2;
		switch (r) {
			case 0: {
				auto t = std::make_shared<AcidItem>(field,x,y);
				t->initSelf(t);
				return std::move(t);
			}
			case 1: {
				auto t = std::make_shared<BombItem>(field,x,y);
				t->initSelf(t);
				return std::move(t);
			}
			default:
				assert(0);
		}
	}
} ;
