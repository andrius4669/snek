#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL.h>
#include "logger.h"
#include "kbsubscriber.h"
#include "kbpublisher.h"
#include "palette.h"

#include <memory>
#include <vector>
#include <queue>

#define FIELDX 128
#define FIELDY 80
#define CELLXSIZE 8
#define CELLYSIZE 8

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

struct PlayerHead;

struct ItemObject: FieldObject {
	virtual int getHardness() const { return 0; }
	virtual void onStep(std::shared_ptr<PlayerHead> player) = 0;
} ;

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
		ssize_t maxx = x + count;
		ssize_t miny = y - count;
		ssize_t maxy = y + count;
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
		}
	}
} ;

struct QuitNotifier: KeyboardSubscriber {
	virtual void update(KeyboardSubscriber::key_t scan)
	{
		quit = true;
	}

	void set()
	{
		quit = true;
	}
	
	bool isset() const { return quit; }
private:
	bool quit = false;
} ;

static std::vector<std::shared_ptr<FieldObject>> objs;

static void advance()
{
	for (size_t i = 0;i < objs.size();++i) {
		objs[i]->advance();
	}
}

struct PaletteVal {
	unsigned char r,g,b;
} ;

static PaletteVal DefaultPaletteVal = {0,0,0};

static std::vector<PaletteVal> Palette;
static std::vector<int> SnakePalette;

static void initPalette()
{
	// TODO read from config
	
	Palette.push_back({15,15,0});
	Palette.push_back({240,255,255});
	
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({255,0,0});
	Palette.push_back({127,0,0});
	Palette.push_back({200,0,0});
	Palette.push_back({100,0,0});
	
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({0,255,0});
	Palette.push_back({0,127,0});
	Palette.push_back({0,200,0});
	Palette.push_back({0,100,0});
	
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({0,0,255});
	Palette.push_back({0,0,127});
	Palette.push_back({0,0,200});
	Palette.push_back({0,0,100});
	
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({255,255,0});
	Palette.push_back({127,127,0});
	Palette.push_back({200,200,0});
	Palette.push_back({100,100,0});
	
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({255,0,255});
	Palette.push_back({127,0,127});
	Palette.push_back({200,0,200});
	Palette.push_back({100,0,100});
	
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({0,255,255});
	Palette.push_back({0,127,127});
	Palette.push_back({0,200,200});
	Palette.push_back({0,100,100});
	
	Palette.push_back({0,255,63});
	Palette.push_back({255,63,0});
}

static void drawField(SDL_Surface *surf,const std::shared_ptr<Field> &field)
{
	SDL_Rect rect;
	memset(&rect,0,sizeof(rect));
	rect.w = CELLXSIZE;
	rect.h = CELLYSIZE;
	for (size_t y = 0;y < field->fheight;++y) {
		for (size_t x = 0;x < field->fwidth;++x) {
			auto t = field->access(x,y)->getTexture();
			assert(t >= 0 && t < Palette.size());
			auto &pv = Palette[t];
			rect.x = x * CELLXSIZE;
			rect.y = y * CELLYSIZE;
			int r = SDL_FillRect(surf,&rect,SDL_MapRGB(surf->format,pv.r,pv.g,pv.b));
			assert(r == 0);
		}
	}
}

static void drawMenu()
{
	// TODO
}

static void startgame(std::shared_ptr<Field> &field)
{
	auto &l = GLog::getInstance();
	l.log("on startgame\n");
	assert(SnakePalette.size() >= 1);
	KeyboardMapper map(
		SDL_SCANCODE_A,
		SDL_SCANCODE_D,
		SDL_SCANCODE_W,
		SDL_SCANCODE_S); // WASD
	auto snek = std::make_shared<PlayerHead>(SnakePalette[0],5,5,field,map);
	snek->initSelf(snek);
	objs.push_back(snek);
	
	auto gm = std::make_shared<GameMaster>(field);
	objs.push_back(gm);
	
	l.log("after startgame\n");
}

static void redraw(SDL_Window *w,std::shared_ptr<Field> &field,bool force)
{
	if (force || field->isChanged()) {
		SDL_Surface *surf = SDL_GetWindowSurface(w); 
		assert(surf != nullptr);

		drawField(surf,field);
		field->clearChanged();

		SDL_UpdateWindowSurface(w);
	}
}



int main(int argc,char **argv)
{
	auto &l = GLog::getInstance();
	
	l.log("hello world\n");
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		l.logf("SDL_Init failed: %s\n",SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	std::shared_ptr<Field> field = std::make_shared<Field>(FIELDX,FIELDY);
	SDL_Window *w = SDL_CreateWindow("SNEK",
		SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
		field->fwidth * CELLXSIZE,field->fheight * CELLYSIZE,SDL_WINDOW_SHOWN);
	if (!w) {
		l.logf("failed to create window: %s\n",SDL_GetError());
		return 1;
	}

	auto wid = SDL_GetWindowID(w);

	auto &k = GKbdControl::getInstance();
	auto qn = std::make_shared<QuitNotifier>();
	k.subscribe(qn,SDL_SCANCODE_ESCAPE);

	initPalette();
	startgame(field);

	bool needredraw = true;

	unsigned int nextadvance = SDL_GetTicks();

	for (;;) {
		SDL_Event ev;
		int st = 0;
		int r;
		// we first use PollEvent
		// then if we find something, continue using PollEvent till we get nothing
		// then if we find nothing, use WaitEventTimeout once
		for (;;) {
			if (st == 0) {
				r = SDL_PollEvent(&ev);
				if (!r) {
					r = SDL_WaitEventTimeout(&ev,5);
					if (!r)
						break;
					st = 1;
				}
				else {
					st = 2;
				}
			}
			else if (st == 1) {
				break;
			}
			else if (st == 2) {
				r = SDL_PollEvent(&ev);
				if (!r)
					break;
			}
			else
				assert(0);

			switch (ev.type) {
				case SDL_QUIT:
					//l.log("quit event received\n");
					qn->set();
					break;
				case SDL_KEYDOWN: {
					//l.log("keydown event received\n");
					k.update(ev.key.keysym.scancode);
					break;
				}
				case SDL_WINDOWEVENT:
					if (ev.window.windowID == wid) {
						switch (ev.window.event) {
							case SDL_WINDOWEVENT_EXPOSED:
							case SDL_WINDOWEVENT_SIZE_CHANGED:
							case SDL_WINDOWEVENT_SHOWN:
								needredraw = true;
								break;
						}
					}
			}
		}

		unsigned int t = SDL_GetTicks();
		while ((int)(t - nextadvance) >= 0) {
			advance();
			nextadvance += 100;
		}

		redraw(w,field,needredraw);

		if (qn->isset())
			break;
	}
	
	SDL_DestroyWindow(w);
	
	return 0;
}
