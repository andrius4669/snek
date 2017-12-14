#pragma once

#include <memory>
#include "palette.h"
#include "itemobject.h"
#include "field.h"

struct AcidItem: ItemObject {
	AcidItem(std::shared_ptr<Field> field,size_t x,size_t y):
		field(field),x(x),y(y)
	{}
	
	virtual void onStep(std::shared_ptr<PlayerHead> player)
	{
		if (!bleaching) {
			auto t = player->getTail();
			dtex = t->getTexture();
			for (size_t y = 0;y < field->fheight;++y) {
				for (size_t x = 0;x < field->fwidth;++x) {
					if (field->access(x,y) == t) {
						field->access(x,y) = self;
					}
				}
			}
			bleaching = true;
			numsteps = 0;
		}
	}
	
	virtual void advance()
	{
		if (bleaching && numsteps >= 5) {
			finished = true;
			for (size_t y = 0;y < field->fheight;++y) {
				for (size_t x = 0;x < field->fwidth;++x) {
					if (this == field->access(x,y).get()) {
						field->access(x,y) = field->getEmpty();
					}
				}
			}
		}
		++numsteps;
	}
	
	virtual bool isDead() const { return finished; }

	virtual bool isActive() const { return bleaching && !finished; }
	
	virtual size_t getTexture() const
	{
		return numsteps & 1 ? dtex : PAL_WHITE;
	}

	void initSelf(std::shared_ptr<AcidItem> s)
	{
		assert(this == s.get());
		self = s;
	}
private:
	std::shared_ptr<Field> field;
	size_t x,y;
	bool finished = false;
	bool bleaching = false;
	size_t numsteps = 0;
	std::shared_ptr<AcidItem> self;
	int dtex = PAL_IT_ACID;
} ;
