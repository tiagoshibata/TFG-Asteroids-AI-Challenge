CXXFLAGS=-O3 -flto -Wall -Wextra -std=c++11
LDFLAGS=-Wl,--as-needed
LDLIBS=-lga `pkg-config --libs opencv`

RM=-rm -rf

.PHONY: all clean
all: genetic score_ocr

clean:
	$(RM) genetic score_ocr

genetic: genetic.cpp

score_ocr: score_ocr.cpp
