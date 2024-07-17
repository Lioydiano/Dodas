REM g++ -std=c++17 -lWinmm -Wall -g -static -o dodas include/sista/ANSI-Settings.cpp include/sista/border.cpp include/sista/coordinates.cpp include/sista/cursor.cpp include/sista/field.cpp include/sista/pawn.cpp dodas.cpp

g++ -std=c++17 -Wall -g -c -static include/sista/ANSI-Settings.cpp include/sista/border.cpp include/sista/coordinates.cpp include/sista/cursor.cpp include/sista/field.cpp include/sista/pawn.cpp
g++ -std=c++17 -Wall -g -c -static dodas.cpp
g++ -std=c++17 -Wall -g -static -o dodas dodas.o ANSI-Settings.o border.o coordinates.o cursor.o pawn.o field.o -lWinmm
rm -f *.o