CXX=g++

DBG=-g -O0
OPTIMIZE=-Ofast -flto
CXXFLAGS=$(OPTIMIZE)
CXXFLAGS_DEFAULT=-Wall -Wextra -std=c++11 -Iinclude/
LDFLAGS=-lm

PREFIX=build
BINARY=$(PREFIX)/shibata

TARGETS:=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(TARGETS))
ALL_OBJECTS:=$(wildcard *.o)
# $< = prerequisito, $@ = alvo:
COMPILE=$(CXX) -c $(CXXFLAGS_DEFAULT) $(CXXFLAGS) $< -o $@
LINK=$(CXX) $(OBJECTS) $(CXXFLAGS_DEFAULT) $(CXXFLAGS) -o $@ $(LDFLAGS)
RM=-rm -rf
MKDIR=mkdir -p
MAKEFILE_PATH:=$(abspath $(lastword $(MAKEFILE_LIST)))
SOURCE_DIR:=$(dir $(MAKEFILE_PATH))

.PHONY: all clean install

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(MKDIR) $(PREFIX)
	$(LINK)

bot_interface.o: bot_interface.cpp bot_interface.h
	$(COMPILE)

BotBase.o: BotBase.cpp BotBase.h bot_interface.h
	$(COMPILE)

Control.o: Control.cpp Control.hpp
	$(COMPILE)

Force.o: Force.cpp Force.hpp Params.hpp
	$(COMPILE)

main.o: main.cpp bot_interface.h Control.hpp ShibataBot.hpp BotBase.h
	$(COMPILE)

ShibataBot.o: ShibataBot.cpp ShibataBot.hpp BotBase.h bot_interface.h Control.hpp \
	Force.hpp Params.hpp Simulation.hpp
	$(COMPILE)

Simulation.o: Simulation.cpp Simulation.hpp Params.hpp bot_interface.h
	$(COMPILE)

clean:
	$(RM) $(ALL_OBJECTS) $(PREFIX)
