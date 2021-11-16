tetris: tetris.c
	$(CC) `pkg-config --cflags --libs sdl2` -lm -Wall -Wextra -pedantic -std=c99 -g tetris.c -o tetris
