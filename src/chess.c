#include <stdbool.h>
#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

static bool wking_moved = true;
static bool wlrook_moved = true;
static bool wrrook_moved = true;
static bool bking_moved = true;
static bool blrook_moved = true;
static bool brrook_moved = true;

static char last_board[64];

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

int signum(int a) {
    if (a < 0) {return -1;} else if (a > 0) { return 1; } else { return 0; }
}

bool case_eq(char a, char b) {
    if (a == ' ' || b == ' ')
        return false;

    return (islower(a) && islower(b)) || (isupper(a) && isupper(b));
}


Move new_move(int from, int to, char piece) {
    Move m = {
        .from = from,
        .to = to,
        .piece = piece,
        .promote = 0,
        .capture = 0,
        .castle = false,
        .check = false,
        .checkmate = false
    };
    return m;
}

char check_line(char *board, int pos, int until, int dr, int dc, int* tpos) {
    int delta = signum(dc) + signum(dr)*8;
    char c;

    while (pos >= 0 && pos < 64) {
        pos += delta;
        if (until == pos)
            break;

        if ((c=board[pos]) != ' ') {
            if (tpos != NULL)
                *tpos = pos;
            return c;
        }
    }

    return 0;
}

bool check_check(char *board, bool white) {
    char king = white? 'K':'k';

    // find the index of the king
    int k_i = 0;
    for (;k_i<64; k_i++)
        if (board[k_i] == king)
            break;

    // Check each direction for a piece of the other
    // color which is able to capture our king
    int pos;
    for (int dr=-1; dr<=1; dr++) {
        for (int dc=-1; dc<=1; dc++) {
            if (dr == 0 && dc == 0)
                break;

            // get the piece that lies on this line
            char piece = check_line(board, k_i, -1, dr, dc, &pos);

            // if it is zero no piece detected
            if (piece == 0)
                break;

            // we can't be taken by one of our pieces
            // if the case is the same for both pieces, skip
            if (case_eq(king, piece))
                break;

            // split out cases for linear and diagonal now
            if (abs(dr) == abs(dc)) {
                // split out cases for pawns vs everything else
                if (tolower(piece) == 'p') {
                    // a pawn on the diagonal must be moving positive
                    // for black and negative for white
                    if (dr != (white? -1: 1))
                        break;

                    // a pawn on the diagonal must be only one space away
                    int delta = dc + dr*8;
                    if (k_i == delta+pos)
                        return true;

                    // otherwise the pawn cannot reach our king
                } else {
                    // The piece must be a bishop or a queen
                    if (tolower(piece) == 'q' || tolower(piece) == 'b')
                        return true;

                    // no other pieces can hit on a diagonal
                }
            } else {
                // on a row the only pieces that can threaten our king
                // are the queen and the rook
                if (tolower(piece) == 'q' || tolower(piece) == 'r')
                    return true;
            }
        }
    }

    // Check each knight slot for threatening our king
    for (int dr=-2; dr<=2; dr++) {
        for (int dc=-2; dc<=2; dc++) {
            // only interested in pairs of 1s and 2s, skip otherwise
            if (abs(dr) == abs(dc) || dr==0 || dc==0)
                break;

            // check the square for a knight of the opposite color
            char tknight = white? 'n': 'N';
            int delta = dc + dr*8;

            if (board[pos+delta] == tknight)
                return true;
        }
    }

    // pretty sure at this point we are safe.
    return false;
}

// do_move attempts to execute a move on the board
//
// returns false if invalid, the board is not changed
// returns true if valid, changing the board and writing the move to out
char* do_move(char *board, int from, int to, bool white, Move *out) {
    char piece = board[from];
    // check that we aren't moving a space
    if (piece == ' ')
        return "cannot move an empty space";

    // check that the piece being moved is of our color
    if (!case_eq(piece, white? 'K' : 'k'))
        return "cannot move enemy piece";

    // now check that the square we are moving to does not
    // contain a piece of our color
    if (case_eq(board[to], piece))
        return "cannot capture friendly piece";

    *out = new_move(from, to, board[from]);
    piece = tolower(board[from]);

    int from_row = from / 8;
    int from_col = from % 8;
    int to_row = to / 8;
    int to_col = to % 8;

    if (piece == 'k') {
        // the king can only move 1 space each direction, except for castline
        if (abs(from_row-to_row) != 1 && abs(from_col-to_col) != 1) {
            // this must be a castle
            out->castle = true;
            int dr = signum(to_row-from_row);

            // check that the king and rook have not moved
            if (white && wking_moved)
                return "cannot castle after king has moved";

            if (white && dr == 1 && wrrook_moved)
                return "cannot castle after rook has moved";

            if (white && dr == -1 && wlrook_moved)
                return "cannot castle after rook has moved";

            if (!white && bking_moved)
                return "cannot castle after king has moved";

            if (!white && dr == 1 && brrook_moved)
                return "cannot castle after rook has moved";

            if (!white && dr == -1 && blrook_moved)
                return "cannot castle after rook has moved";
            
            
            // There are only 4 pairs of from/to values
            // for a castle. We check them here for the correct color
            if (white && from != 60)
                return "invalid king move";

            if (!white && from != 4)
                return "invalid king move";

            if (white && (to != 62 || to != 58))
                return "invalid king move";

            if (!white && (to != 2 || to != 6))
                return "invalid king move";

            // there cannot be anything in the way of the king to the rook
            if (check_line(board, from+dr, to, dr, to_col-from_col, NULL) != 0)
                return "cannot move through piece";

            // the king cannot pass through a check to get there
            char board_copy[64];
            memcpy(board_copy, board, 64);
            int k_i = from;

            while (k_i != to) {
                board_copy[k_i+dr] = board_copy[k_i];
                board_copy[k_i] = ' ';
                k_i++;

                if (check_check(board, white))
                    return "cannot castle through a check";
            }

            // swap the rook
            board_copy[k_i-dr] = board_copy[k_i+dr];
            board_copy[k_i+dr] = ' ';

            memcpy(last_board, board, 64);
            memcpy(board, board_copy, 64);
            return NULL;
        }
    } else if (piece == 'r') {
        // a rook can only move horizontall or vertically
        if (from_row != to_row && from_col != to_col)
            return "invalid rook move";

        // there can't be anything between this rook and its target
        if (check_line(board, from, to, to_row-from_row, to_col-from_col, NULL) != 0)
            return "cannot move through piece";
    } else if (piece == 'b') {
        // a bishop can only move diagonally
        if (abs(from_row-to_row) != abs(from_col-to_col))
            return "invalid bishop move";

        // there can't be anything between this bishop and its target
        if (check_line(board, from, to, to_row-from_row, to_col-from_col, NULL) != 0)
            return "cannot move through piece";
    } else if (piece == 'n') {
        // a knight moves in an lshaped pattern. One delta must be 1, and the other 2
        int drow = abs(from_row-to_row);
        int dcol = abs(from_col-to_col);

        if (drow == 1 && dcol != 2)
            return "invalid knight move";

        if (drow == 2 && dcol != 1)
            return "invalid knight move";

        if (drow != 1 && drow != 2)
            return "invalid knight move";
    } else if (piece == 'q') {
        // a queen can move horizontally or diagonally
        if (abs(from_row-to_row) != abs(from_col-to_col))
            if (from_row != to_row && from_col != to_col)
                return "invalid queen move";

        // there can't be anything between this queen and its target
        if (check_line(board, from, to, to_row-from_row, to_col-from_col, NULL) != 0)
            return "cannot move through piece";
    } else if (piece == 'p') {
        // a pawn must either move forward (optionally promoted),
        // take diagonally, or en-passant
        int diff = abs(to-from);
        int dc = signum(to_col-from_col);

        if (diff == 16 && ((white && from_row==6) || (!white && from_row==1))) {
            // first move is allowed to move two squares. There cannot be anything
            // in front of us or where we are landing
            if (board[to+8*dc] != ' ')
                return "cannot move through pieces";

            if (board[to] != ' ')
                return "pawn cannot capture vertically";
            
        } else if (diff == 8) {
            // this is moving forward one square
            // ensure that no piece is in front of it
            if (board[to] != ' ')
                return "pawn cannot capture vertically";

            // check if this is promoted
            if (to_row == 0 || to_row==7) {
                out->promote = white? 'Q' : 'q';
            }
        } else if (diff == 7 || diff == 9) {
            // this is moving diagonally to capture or en-passant
            // en-passant is a bit tricky to detect. We see if we
            // are on the correct row, there is an enemy pawn
            // that we are passing, and that pawn was on its
            // home square two moves ago
            if ((white && from_row == 3) || (!white && from_row==4)) {
                char enemy = white? 'p':'P';
                if (board[to+8*dc] == enemy) {
                    if (last_board[to+16*dc] == enemy) {
                        // this is a legal en-passant
                        out->capture = to+8*dc;

                        goto next;
                    }
                }
            }

            // with en-passant out of the way, just check for captures
            // we've also already checked that "to" is not our color
            if (board[to] == ' ')
                return "pawn must capture diagonally";
        } else {
            return "invalid pawn move";
        }
    }

    next:

    // if our to square is not empty we are capturing it
    if (board[to] != ' ')
        out->capture = to;

    // Now all checks have been completed (except check_check),
    // so we execute the move as we have populated the move struct
    // on a copy of the board
    char board_copy[64];
    memcpy(board_copy, board, 64);

    // do this move on a copy
    // ensure that we are not in check after this move.
    if (out->capture)
        board_copy[out->capture] = ' ';

    board_copy[out->to] = board_copy[out->from];
    board_copy[out->from] = ' ';

    if (out->promote)
        board_copy[out->to] = out->promote;
    
    if (check_check(board_copy, white))
        return "cannot leave king in check";

    // set whether the other king is in check
    out->check = check_check(board_copy, !white);
    // TODO
    // if (out->check)
        // out->checkmate = check_checkmate(board_copy, !white);

    // copy back and return, this was a valid move
    memcpy(last_board, board, 64);
    memcpy(board, board_copy, 64);
    return NULL;
}

