tetris: tetris.c
	$(CC) `pkg-config --cflags --libs sdl2` -lm -Wall -Wextra -pedantic -std=c99 -g tetris.c -o tetris

CFLAGS_W = -lm -Wextra -pedantic -std=c99 -Wall -Wl,-subsystem,console
LFLAGS_W = -lmingw32 -lSDL2main -lSDL2
INCS_W = -I/usr/x86_64-w64-mingw32/sys-root/mingw/include/SDL2
LIBS_W = -L/usr/x86_64-w64-mingw32/sys-root/mingw/lib

windows:
	x86_64-w64-mingw32-gcc tetris.c $(INCS_W) $(LIBS_W) $(CFLAGS_W) $(LFLAGS_W) -g -o bin/tetris.exe
