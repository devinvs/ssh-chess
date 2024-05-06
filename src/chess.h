#ifndef _CHESS_H
#define _CHESS_H

#include <stdbool.h>

typedef struct {
    int from;
    int to;
    char piece;
    char promote;
    int capture;
    bool castle: 1;
    bool check: 1;
    bool checkmate: 1;
} Move;

Move new_move(int from, int to, char piece);
char check_line(char *board, int pos, int until, int dr, int dc, int* tpos);
bool check_check(char *board, bool white);
char* check_move(char *board, Move last, int from, int to, bool white, Move *out);
void do_move(char *board, Move m);

#endif
