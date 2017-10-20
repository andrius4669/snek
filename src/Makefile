MV=mv

CXXSTD= -std=c++11
CXXFLAGS= $(CXXSTD) -O2
CXXINCLUDE= `sdl2-config --cflags`
LDFLAGS= `sdl2-config --libs`
CLIENTOBJ= \
	client.o \
	logger.o \
	config.o

ALLO= $(sort $(CLIENTOBJ))
ALLC= $(patsubst %.o,%.cpp,$(filter %.o %.cpp,$(ALLO)))
CLEANO= $(filter %.o,$(ALLO))
EXE= snekc

default: snekc

all: $(EXE)

snekc: $(CLIENTOBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CXXINCLUDE) -c -o $@ $<

clean:
	$(RM) $(CLEANO)
	$(RM) $(EXE)

depend:
	makedepend -Y -- $(CXXSTD) -- $(ALLC)

# DO NOT DELETE THIS LINE

client.o: logger.h singleton.h
config.o: config.h singleton.h
logger.o: logger.h singleton.h config.h
