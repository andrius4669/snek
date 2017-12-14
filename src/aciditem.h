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
		if (state == ST_WAITING) {
			auto t = player->getTail();
			dtex = t->getTexture();
			for (size_t y = 0;y < field->fheight;++y) {
				for (size_t x = 0;x < field->fwidth;++x) {
					if (field->access(x,y) == t) {
						field->access(x,y) = self;
					}
				}
			}
			state = ST_BLEACHING;
			numsteps = 0;
		}
	}
	
	virtual void advance()
	{
		if (state == ST_BLEACHING && numsteps >= 5) {
			state = ST_FINISHED;
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
	
	virtual bool isDead() const { return state == ST_FINISHED; }

	virtual bool isActive() const { return state == ST_BLEACHING; }
	
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
	enum {
		ST_WAITING,
		ST_BLEACHING,
		ST_FINISHED,
	} ;

	std::shared_ptr<Field> field;
	size_t x,y;
	int state = ST_WAITING;
	size_t numsteps = 0;
	std::shared_ptr<AcidItem> self;
	int dtex = PAL_IT_ACID;
} ;
