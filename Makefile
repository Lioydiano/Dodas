# Makefile for dodas.cpp
IMPLEMENTATIONS = include/sista/ANSI-Settings.cpp include/sista/border.cpp include/sista/coordinates.cpp include/sista/cursor.cpp include/sista/field.cpp include/sista/pawn.cpp

all:
	g++ -std=c++17 -Wall -g -c $(IMPLEMENTATIONS)
	g++ -std=c++17 -Wall -g -c dodas.cpp
	g++ -std=c++17 -Wall -g -o dodas dodas.o ANSI-Settings.o border.o coordinates.o cursor.o pawn.o field.o
	rm -f *.o