#ifndef _GAME_H
#define _GAME_H

typedef struct game_s {
    char **moves;
    int num_moves;
    int cap_moves;
    char board[64];
}* Game;

Game new_game(void);
void free_game(Game);
void push_move(Game, char*);

#endif
