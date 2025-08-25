UNAME := $(shell uname)
STATIC_FLAG =
WINMM_FLAG =
ifeq ($(UNAME),Darwin)
    STATIC_FLAG =
else
    STATIC_FLAG = -static
endif

ifeq ($(OS),Windows_NT)
    WINMM_FLAG = -lwinmm
endif

all:
	g++ -std=c++17 -Wall -g $(STATIC_FLAG) -c dodas.cpp
	g++ -std=c++17 -Wall -g $(STATIC_FLAG) -o dodas dodas.o $(WINMM_FLAG) -lSista
	rm -f *.o
