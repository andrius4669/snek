#pragma once

#include "palette.h"
#include "itemobject.h"
#include "field.h"

struct BombItem: ItemObject {
	BombItem(std::shared_ptr<Field> field,size_t x,size_t y):
		field(field),x(x),y(y)
	{}
	
	virtual void onStep(std::shared_ptr<PlayerHead> player)
	{
		if (!explode) {
			dtex = player->getTexture();
			explode = true;
			doArea(0,true);
			doArea(1,true);
			numsteps = 2;
		}
	}
	
	virtual void advance()
	{
		if (explode) {
			if (numsteps <= 4)
				doArea(numsteps - 2,false);
			if (numsteps >= 4 && !doArea(numsteps - 1,false))
				finished = true;
			doArea(numsteps,true);
		}
		++numsteps;
	}
	
	virtual bool isDead() const { return finished; }
	virtual bool isActive() const { return explode && !finished; }
	
	virtual size_t getTexture() const
	{
		if (explode) {
			if (numsteps > 4)
				return dtex;
			return PAL_WHITE;
		}
		return numsteps & 1 ? dtex : PAL_WHITE;
	}
	
	void initSelf(std::shared_ptr<BombItem> s)
	{
		assert(this == s.get());
		self = s;
	}
private:
	std::shared_ptr<Field> field;
	size_t x,y;
	bool finished = false;
	bool explode = false;
	size_t numsteps = 0;
	std::shared_ptr<BombItem> self;
	int dtex = PAL_IT_BOMB; // distinct texture

	inline bool shouldPad(size_t x,size_t y)
	{
		auto v = field->access(x,y).get();
		if (v->getHardness() < 0)
			return true;
		auto t = dynamic_cast<PlayerTail *>(v);
		if (t != nullptr)
			return true;
		// allow padding on top of other explosions
		auto b = dynamic_cast<BombItem *>(v);
		return b != nullptr && b->explode;
	}

	inline bool shouldClean(size_t x,size_t y)
	{
		return this == field->access(x,y).get();
	}
	
	bool doArea(size_t count,bool pad)
	{
		ssize_t minx = x - count;
		size_t maxx = x + count;
		ssize_t miny = y - count;
		size_t maxy = y + count;
		// upper line
		if (miny >= 0) {
			auto ty = miny;
			auto mmaxx = maxx < field->fwidth ? maxx : field->fwidth - 1;
			for (size_t tx = minx >= 0 ? minx : 0;tx <= mmaxx;++tx) {
				if (pad) {
					if (shouldPad(tx,ty))
						field->access(tx,ty) = self;
				} else {
					if (shouldClean(tx,ty))
						field->access(tx,ty) = field->getEmpty();
				}
			}
		}
		// lower line
		if (maxy < field->fheight) {
			auto ty = maxy;
			auto mmaxx = maxx < field->fwidth ? maxx : field->fwidth - 1;
			for (size_t tx = minx >= 0 ? minx : 0;tx <= mmaxx;++tx) {
				if (pad) {
					if (shouldPad(tx,ty))
						field->access(tx,ty) = self;
				} else {
					if (shouldClean(tx,ty))
						field->access(tx,ty) = field->getEmpty();
				}
			}
		}
		// left
		if (minx >= 0) {
			auto tx = minx;
			auto mmaxy = maxy < field->fheight ? maxy : field->fheight - 1;
			for (size_t ty = miny >= 0 ? miny : 0;ty <= mmaxy;++ty) {
				if (pad) {
					if (shouldPad(tx,ty))
						field->access(tx,ty) = self;
				} else {
					if (shouldClean(tx,ty))
						field->access(tx,ty) = field->getEmpty();
				}
			}
		}
		// right
		if (maxx < field->fwidth) {
			auto tx = maxx;
			auto mmaxy = maxy < field->fheight ? maxy : field->fheight - 1;
			for (size_t ty = miny >= 0 ? miny : 0;ty <= mmaxy;++ty) {
				if (pad) {
					if (shouldPad(tx,ty))
						field->access(tx,ty) = self;
				} else {
					if (shouldClean(tx,ty))
						field->access(tx,ty) = field->getEmpty();
				}
			}
		}
		return minx >= 0 ||
			maxx < field->fwidth ||
			miny >= 0 ||
			maxy < field->fheight;
	}
} ;
