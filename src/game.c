#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "chess.h"

Game new_game() {
    Game g = malloc(sizeof(struct game_s));

    g->moves = malloc(20 * sizeof(Move));
    g->num_moves = 0;
    g->cap_moves = 20;
    memcpy(g->board, "rnbqkbnrpppppppp                                PPPPPPPPRNBQKBNR", 64);

    return g;
}

void free_game(Game g) {
    free(g->moves);
    free(g);
}

void push_move(Game g, Move m) {
    if (g->num_moves == g->cap_moves) {
        g->cap_moves = g->cap_moves*2+1;
        g->moves = realloc(g->moves, g->cap_moves * sizeof(Move));
    }
    
    memcpy(g->moves + g->num_moves, &m, sizeof(Move));
    g->num_moves++;
}
