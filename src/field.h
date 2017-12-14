#pragma once

#include "fieldobject.h"

struct Field {
	typedef std::shared_ptr<FieldObject> FOPtr;

	Field() = delete;
	Field(const Field &) = delete;
	Field &operator=(const Field &) = delete;

	Field(size_t width,size_t height):
		fwidth(width),fheight(height),
		field(width * height),
		emptyobj(std::make_shared<EmptyFieldObject>()),
		changed(true)
	{
		for (size_t i = 0;i < field.size();++i) {
			field[i] = emptyobj;
		}
	}
	
	const size_t fwidth,fheight;
	
	FOPtr &access(size_t x, size_t y)
	{
		assert(x < fwidth);
		assert(y < fheight);
		return field[fwidth * y + x];
	}
	const FOPtr &access(size_t x, size_t y) const
	{
		assert(x < fwidth);
		assert(y < fheight);
		return field[fwidth * y + x];
	}
	
	FOPtr getEmpty() { return emptyobj; }

	inline void setChanged() { changed = true; }
	inline void clearChanged() { changed = false; }
	inline bool isChanged() const { return changed; }
private:
	std::vector<FOPtr> field;
	FOPtr emptyobj;
	bool changed;
} ;
