#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL.h>
#include "logger.h"
#include "kbsubscriber.h"
#include "kbpublisher.h"

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
	
	virtual bool isDead() const = 0;
	
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
		auto i = fwidth * y + x;
		assert(i < field.size());
		return field[i];
	}
	const FOPtr &access(size_t x, size_t y) const
	{
		auto i = fwidth * y + x;
		assert(i < field.size());
		return field[i];
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

struct PlayerHead: FieldObject,KeyboardSubscriber {
	virtual size_t getTexture() const { return texture; }
	virtual int getHardness() const { return 1; }
	virtual bool isDead() const { return dead; }

	virtual void update(KeyboardSubscriber::key_t scan)
	{
		int dir = kbmapper.getDirection(scan);
		assert(dir >= 0);
		auto &l = GLog::getInstance();
		l.logf("snake received direction %d\n",dir);
		if (!directions.empty() && directions.back() == dir)
			return;
		if (dead)
			return;
		directions.push(dir);
	}

	virtual bool canAdvance() const { return !dead; }
	virtual void advance()
	{
		if (directions.empty())
			return;
		int dir = directions.front();
		if (directions.size() > 1)
			directions.pop();
		if (dead)
			return;

		auto &l = GLog::getInstance();
		l.logf("advance with direction %d\n",dir);

		size_t newx = x,newy = y;
		assert(!!field);
		switch (dir) {
			case DIR_LEFT:
				newx = (x - 1) % field->fwidth;
				break;
			case DIR_RIGHT:
				newx = (x + 1) % field->fwidth;
				break;
			case DIR_UP:
				newy = (y - 1) % field->fheight;
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

private:
	int texture;
	std::shared_ptr<FieldObject> tail;
	std::queue<int> directions;
	size_t x,y;
	std::shared_ptr<Field> field;
	bool dead;
	std::shared_ptr<FieldObject> self;
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

std::vector<std::shared_ptr<FieldObject>> objs;

static void advance()
{
	for (size_t i = 0;i < objs.size();++i) {
		objs[i]->advance();
	}
}

struct PaletteVal {
	unsigned char r,g,b;
} ;

PaletteVal DefaultPaletteVal = {0,0,0};

std::vector<PaletteVal> Palette;
std::vector<int> SnakePalette;

void initPalette()
{
	// TODO read from config
	// empty
	Palette.push_back({15,15,0});
	// snake 0
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({255,0,0});
	Palette.push_back({127,0,0});
	Palette.push_back({200,0,0});
	Palette.push_back({100,0,0});
	// snake 1
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({0,255,0});
	Palette.push_back({0,127,0});
	Palette.push_back({0,200,0});
	Palette.push_back({0,100,0});
	// snake 2
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({0,0,255});
	Palette.push_back({0,0,127});
	Palette.push_back({0,0,200});
	Palette.push_back({0,0,100});
	// snake 3
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({255,255,0});
	Palette.push_back({127,127,0});
	Palette.push_back({200,200,0});
	Palette.push_back({100,100,0});
	// snake 4
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({255,0,255});
	Palette.push_back({127,0,127});
	Palette.push_back({200,0,200});
	Palette.push_back({100,0,100});
	// snake 5
	SnakePalette.push_back((int)Palette.size());
	Palette.push_back({0,255,255});
	Palette.push_back({0,127,127});
	Palette.push_back({0,200,200});
	Palette.push_back({0,100,100});
}

void drawField(SDL_Surface *surf,const std::shared_ptr<Field> &field)
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

void drawMenu()
{
	// TODO
}

void startgame(std::shared_ptr<Field> &field)
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
	l.log("after startgame\n");
}

void redraw(SDL_Window *w,std::shared_ptr<Field> &field)
{
	if (field->isChanged()) {
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
	
	auto &k = GKbdControl::getInstance();
	auto qn = std::make_shared<QuitNotifier>();
	k.subscribe(qn,SDL_SCANCODE_ESCAPE);
	
	initPalette();
	startgame(field);
	redraw(w,field);
	
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
					l.log("quit event received\n");
					qn->set();
					break;
				case SDL_KEYDOWN: {
					l.log("keydown event received\n");
					k.update(ev.key.keysym.scancode);
					break;
				}
			}
		}
		
		unsigned int t = SDL_GetTicks();
		if ((int)(t - nextadvance) >= 0) {
			advance();
			nextadvance += 100;
		}
		
		redraw(w,field);
		if (qn->isset())
			break;
	}
	
	SDL_DestroyWindow(w);
	
	return 0;
}
