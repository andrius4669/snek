#pragma once

#include "fieldobject.h"
#include "playerhead.h"
#include <memory>

struct PlayerHead;

struct ItemObject: FieldObject {
	virtual int getHardness() const { return 0; }
	virtual void onStep(std::shared_ptr<PlayerHead> player) = 0;
} ;
