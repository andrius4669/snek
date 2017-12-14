#pragma once

struct FieldObject {
	virtual size_t getTexture() const = 0;
	
	virtual int getHardness() const = 0;
	
	virtual bool canAdvance() const
	{
		return false;
	}
	
	virtual void advance() {}

	// whether can be reaped
	virtual bool isDead() const = 0;

	// represents activity
	virtual bool isActive() const { return false; }
	
	virtual ~FieldObject() {}
} ;

struct EmptyFieldObject: FieldObject {
	virtual size_t getTexture() const { return 0; }
	virtual int getHardness() const { return -1; }
	virtual bool isDead() const { return true; }
} ;
