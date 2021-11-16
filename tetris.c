#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdlib.h>
#include <time.h>

#define COL 10
#define ROW 30

long ticks() 
{
    return SDL_GetTicks();
}

const int shape_i[4][4] = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {1, 1, 1, 1},
    {0, 0, 0, 0}
};

const int shape_o[4][4] = {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
};

const int shape_j[4][4] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0}
};

const int shape_l[4][4] = {
    {0, 0, 1, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

const int shape_s[4][4] = {
    {0, 1, 1, 0},
    {1, 1, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

const int shape_z[4][4] = {
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

const int shape_t[4][4] = {
    {0, 1, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

const int (*shapes[7])[4][4] = {
    &shape_i,
    &shape_o,
    &shape_l,
    &shape_j,
    &shape_s,
    &shape_z,
    &shape_t,
};

struct Piece {
    int shape[4][4];
    int x;
    int y;
    int id;
};

struct KeyTable {
    int right;
    int left;
    int rot;
    int drop;
    int fast;
};

void draw(SDL_Renderer *renderer, int *map, struct Piece piece) 
{
    // bg
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    // frame
    SDL_Rect frame = {0, 0, COL*14, ROW*14};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &frame);

    // map
    SDL_Rect square = {0, 0, 12, 12};
    for (int row=0; row < ROW; row++) {
        for (int col=0; col < COL; col++) {
            int color = map[row*COL+col];
            square.y = row*14 + 1;
            square.x = col*14 + 1;
            if (color != 0) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &square);
            }
        }
    }
    // piece 
    for (int row=0; row < 4; row++) {
        for (int col=0; col < 4; col++) {
            int color = piece.shape[row][col];
            square.y = (row + piece.y)*14 + 1;
            square.x = (col + piece.x)*14 + 1;
            if (color != 0) {
                SDL_SetRenderDrawColor(renderer, 255, 100, 255, 255);
                SDL_RenderDrawRect(renderer, &square);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

int rotate(struct Piece *piece)
{
    struct Piece tmp;
    memcpy(&tmp, piece, sizeof(tmp));
    int n = 3;
    if (piece->id == 0)
        n = 4; // shape_i
    for (int row=0; row < n; row++) {
        for (int col=0; col < n; col++) {
            piece->shape[row][col] = tmp.shape[n - col - 1][row];
        }
    }
    return 0;
}

int colliding(const int *map, const struct Piece *piece) 
{
    for (int row=0; row < 4; row++) {
        for (int col=0; col < 4; col++) {
            int mapx = piece->x + col;
            int mapy = piece->y + row;
            if (mapy < 0 ||
                (piece->shape[row][col] &&
                 (map[mapy*COL+mapx] ||
                   mapx < 0 || mapy >= ROW || mapx >= COL)))
                return 1;
        }
    }
    return 0;
}

int place_on_map(int *map, struct Piece *piece) 
{
    for (int row=0; row < 4; row++) {
        for (int col=0; col < 4; col++) {
            int mapx = piece->x + col;
            int mapy = piece->y + row;
            int tile = piece->shape[row][col];
            if (tile)
                map[mapy*COL+mapx] = tile;
        }
    }
    return 0;
}

void get_piece(struct Piece *piece) 
{
    piece->id = rand() % 7;
    memcpy(piece->shape, shapes[piece->id], sizeof(piece->shape));
}

void drop_line(int *map, int n) 
{
    for (int row=n; row > 0; row--) {
        if (row != ROW)
            memcpy(&map[row*COL], &map[(row-1)*COL], sizeof(int)*COL);
    }
}

int clear_lines(int *map) 
{
    int n_lines = 0;
    for (int row=0; row < ROW; row++) {
        int good = 1;
        for (int col=0; col < COL; col++) {
            if (!map[row*COL+col])
                good = 0;
        }
        if (good) {
            drop_line(map, row);
            n_lines++;
        }
    }
    return n_lines;
}

int step(int *map, struct Piece *piece, int *cleared_lines) 
{
    piece->y += 1;
    
    if (!colliding(map, piece))
        return 0;
    
    piece->y -= 1;
    place_on_map(map, piece);
    *cleared_lines = clear_lines(map);

    // new piece
    piece->x = (COL/2-2);
    piece->y = 0;
    get_piece(piece);

    if (colliding(map, piece))
        return 2;
    
    return 1;
}

int handle_input(struct KeyTable *key) 
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            return 1;
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_RIGHT:
                    key->right = 1;
                    break;
                case SDLK_LEFT:
                    key->left = 1;
                    break;
                case SDLK_UP:
                    key->rot = 1;
                    break;
                case SDLK_SPACE:
                    key->drop = 1;
                    break;
                case SDLK_DOWN:
                    key->fast = 1;
                    break;
            }
        }
        if (e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
                case SDLK_RIGHT:
                    key->right = 0;
                    break;
                case SDLK_LEFT:
                    key->left = 0;
                    break;
                case SDLK_UP:
                    key->rot = 0;
                    break;
                case SDLK_DOWN:
                    key->fast = 0;
                    break;
            }
        }
    }
    return 0;
}

int main() 
{
    srand(time(NULL));
    int step_time = 500; // fall down a tile every step_time
    int input_step_time = 1000/15; // apply input at 15 fps
    long next_step = ticks() + step_time;
    long next_input_step = ticks() + input_step_time;
    int score = 0;
    int cleared_lines;

    int map[ROW*COL] = {0};
    struct Piece piece = {{{0}}, (COL/2-2), 0, 0};
    get_piece(&piece);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow(
            "Tetris",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            640, 480,
            SDL_WINDOW_SHOWN
            );
    SDL_Renderer *renderer = SDL_CreateRenderer(
            window, -1, SDL_RENDERER_SOFTWARE);

    struct KeyTable key_table = {0};

    while (1) {
        if (handle_input(&key_table) == 1)
            break;
        if (ticks() > next_input_step) {
            next_input_step = ticks() + input_step_time;
            if (key_table.right) {
                piece.x += 1;
                if (colliding(map, &piece))
                    piece.x -= 1;
            }
            if (key_table.left) {
                piece.x -= 1;
                if (colliding(map, &piece))
                    piece.x += 1;
            }
            if (key_table.rot) {
                struct Piece tmp;
                memcpy(&tmp, &piece, sizeof(tmp));
                rotate(&piece);
                if (colliding(map, &piece))
                    memcpy(&piece, &tmp, sizeof(piece));
                key_table.rot = 0;
            }
        }
        if (key_table.drop || ticks() > next_step -
                (key_table.fast ? (7 * step_time / 8) : 0)) {
            int step_r = step(map, &piece, &cleared_lines);
            if (step_r) {
                if (step_time > 120)
                    step_time -= 20;
                key_table.drop = 0;
            }
            if (step_r == 2) {
                puts("game over");
                break;
            }
            next_step = ticks() + step_time;
        }
        // score
        switch (cleared_lines) {
            case 1:
                score += 40;
                break;
            case 2:
                score += 100;
                break;
            case 3:
                score += 300;
                break;
            case 4:
                score += 1200;
                break;
        }
        // drawing
        draw(renderer, map, piece);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
