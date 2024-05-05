#ifndef _CHESS_H
#define _CHESS_H

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
char* do_move(char *board, int from, int to, bool white, Move *out);

#endif
