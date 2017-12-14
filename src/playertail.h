#pragma once

#include "fieldobject.h"
#include "field.h"

struct PlayerTail: FieldObject {
	virtual size_t getTexture() const { return texture; }
	virtual int getHardness() const { return 1; }
	virtual bool isDead() const { return dead; }
	void died()
	{
		if (!dead) {
			dead = true;
			++texture;
		}
	}
	
	PlayerTail(int txt): texture(txt),dead(false) {}
private:
	int texture;
	bool dead;
} ;

enum {
	DIR_LEFT = 0,
	DIR_RIGHT,
	DIR_UP,
	DIR_DOWN,
	DIR_NUM
} ;
