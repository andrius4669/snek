MV=mv

CXXSTD= -std=c++11 -Wall
CXXFLAGS= $(CXXSTD) -Og -ggdb3 -fno-inline
CXXINCLUDE= `sdl2-config --cflags`
LDFLAGS= `sdl2-config --libs`
CLIENTOBJ= \
	client.o \
	logger.o \
	config.o \
	kbpublisher.o

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

client.o: logger.h kbsubscriber.h kbpublisher.h
config.o: config.h cpptoml/include/cpptoml.h
kbpublisher.o: kbpublisher.h kbsubscriber.h logger.h
logger.o: logger.h config.h cpptoml/include/cpptoml.h
