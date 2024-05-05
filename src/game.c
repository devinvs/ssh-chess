#include <stdlib.h>
#include <string.h>

#include "game.h"

Game new_game() {
    Game g = malloc(sizeof(struct game_s));

    g->moves = malloc(20 * sizeof(char *));
    g->num_moves = 0;
    g->cap_moves = 20;
    memcpy(g->board, "rnbqkbnrpppppppp                                PPPPPPPPRNBQKBNR", 64);

    return g;
}

void free_game(Game g) {
    // free the moves list
    for (int i=0; i<g->num_moves; i++) {
        free(g->moves[i]);
    }
    free(g->moves);
    free(g);
}

void push_move(Game g, char *move) {
    if (g->num_moves == g->cap_moves) {
        g->cap_moves = g->cap_moves*2+1;
        g->moves = realloc(g->moves, g->cap_moves * sizeof(char *));
    }
    
    size_t slen = strlen(move);
    char *buf = malloc(slen+1);
    strcpy(buf, move);

    g->moves[g->num_moves] = buf;
    g->num_moves++;
}
