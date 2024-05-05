#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/select.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>

#include "term.h"
#include "game.h"
#include "chess.h"

#define RED 186, 68, 76
#define GREEN 95, 126, 50
#define BLUE 55, 114, 190
#define PURPLE 147, 74, 190

static bool white = true;

// some global variables for querying the size of the window
// and other important ui anchor points
static struct winsize s;
int center_col;
int center_row;
int row_off;

// global state of the ui input and scroll
static unsigned char input[21] = {0};
static int input_i = 0;
static int scroll = 0;

// TODO: this shouldn't be global
static Game g;

void bg_board() {
    set_bg(130, 101, 195);
}

struct winsize window_size() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w;
}

void print_warning(char *msg) {
    bold();
    fg_red();
    move_cursor(row_off+24, center_col-7);
    printf("                                          ");
    move_cursor(row_off+24, center_col-7);
    printf(msg);
    reset();
    fflush(0);
}

void print_board(bool flip, int row, int col) {
    move_cursor(row, col);
    printf("▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁");
    row++;

    move_cursor(row, col);
    bg_board();
    printf("▎");
    fg_gray();
    printf("  ┌───┬───┬───┬───┬───┬───┬───┬───┐ ");
    reset();
    row++;

    for (int i=0; i<8; i++) {
        move_cursor(row, col);
        // printf("▕");
        bg_board();
        int r = i;
        if (flip) {
            r = 7-i;
        }

        printf("▎");
        fg_gray();
        putchar("87654321"[r]);
        putchar(' ');
        printf("│");

        for (int j=0; j<8; j++) {
            int c = j;
            if (flip) {
                c = 7-j;
            }
            
            char out = g->board[r*8 + c];

            switch (out) {
                // white pieces
                case 'P':
                    fg_white();
                    printf(" ♟︎ ");
                    break;
                case 'R':
                    fg_white();
                    printf(" ♜ ");
                    break;
                case 'N':
                    fg_white();
                    printf(" ♞ ");
                    break;
                case 'B':
                    fg_white();
                    printf(" ♝ ");
                    break;
                case 'Q':
                    fg_white();
                    printf(" ♛ ");
                    break;
                case 'K':
                    fg_white();
                    printf(" ♚ ");
                    break;
                // black pieces
                case 'p':
                    fg_black();
                    // printf(" ♙ ");
                    printf(" ♟︎ ");
                    break;
                case 'r':
                    fg_black();
                    // printf(" ♖ ");
                    printf(" ♜ ");
                    break;
                case 'n':
                    fg_black();
                    // printf(" ♘ ");
                    printf(" ♞ ");
                    break;
                case 'b':
                    fg_black();
                    // printf(" ♗ ");
                    printf(" ♝ ");
                    break;
                case 'q':
                    fg_black();
                    // printf(" ♕ ");
                    printf(" ♛ ");
                    break;
                case 'k':
                    fg_black();
                    // printf(" ♔ ");
                    printf(" ♚ ");
                    break;
                default:
                    printf("   ");
                    break;
            }

            reset();
            bg_board();
            fg_gray();
            printf("│");
        }
        printf(" ");
        row++;
        reset();

        move_cursor(row, col);
        bg_board();

        if (i != 7) {
            printf("▎");
            fg_gray();
            printf("  ├───┼───┼───┼───┼───┼───┼───┼───┤ ");
        } else {
            printf("▎");
            fg_gray();
            printf("  └───┴───┴───┴───┴───┴───┴───┴───┘ ");
            reset();
            row++;
            move_cursor(row, col);
            bg_board();
            if (flip) {
                printf("▎");
                fg_gray();
                printf("    h   g   f   e   d   c   b   a   ");
            } else {
                printf("▎");
                fg_gray();
                printf("    a   b   c   d   e   f   g   h   ");
            }
        }
        reset();
        row++;
    }

    move_cursor(row, col);
    printf("▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔");
}

void draw_history(int row, int col, char** moves, int n) {
    int init_row = row;

    int start;
    int num;

    if ( n < 18) {
        start = 0;
        num = n;
    } else {
        
    }

    bool white = true;
    for (;start<num; start++) {
        if (white) {
            move_cursor(row, col);
            printf("%02d.  ", (start/2)+1);
        } else {
            move_cursor(row, col+21);
            row++;
        }
        printf("%s", moves[start]);
        white = !white;
    }

    dim();
    move_cursor(init_row+17, col+8);
    printf("🞁/🞃 to scroll");
    reset();
}

void print_input(int row, int col) {
    move_cursor(row, col);
    printf("your move:");
    move_cursor(row+1, col);
    printf("┏━━━━━━━━━━━━━━━━━━━━┓");
    move_cursor(row+2, col);
    printf("┃");
    if (input_i > 0)
        printf("%s", input);
    move_cursor(row+2, col+21);
    printf("┃");
    move_cursor(row+3, col);
    printf("┗━━━━━━━━━━━━━━━━━━━━┛");

}

void vline(int row, int col, int len) {
    for (int i=0; i<=len; i++) {
        move_cursor(row+i, col);
        printf("▎");
    }
}

void hline(int row, int col, int len) {
    for (int i=0; i<=len; i++) {
        move_cursor(row, col+i);
        printf("▔");
    }
}

void draw_game_screen() {
    clear();

    if (s.ws_row < 24) {
        move_cursor(center_row, center_col-9);
        printf("terminal too small");
        move_cursor(center_row+1, center_col-8);
        printf("resize to 24 rows");
        fflush(0);
        return;
    }

    if (s.ws_col < 80) {
        move_cursor(center_row, center_col-9);
        printf("terminal too small");
        move_cursor(center_row+1, center_col-10);
        printf("resize to 80 columns");
        fflush(0);
        return;
    }


    // board
    print_board(false, row_off+1, center_col-30);
    vline(row_off+0, center_col+42-5-30, 20);
    hline(row_off+20, center_col-40, 80);

    // usernames
    dim();
    move_cursor(row_off+1, center_col+12);
    printf("N00bSl4yer20 vs Dumbsavanta");
    reset();

    char * moves[5] = {
        "hello",
        "world",
        "i'm",
        "here",
        "to"
    };

    draw_history(row_off+2, center_col+9, moves, 5);

    // move hint text
    move_cursor(row_off+21, center_col-2);
    dim();
    underline();
    printf("move formats");
    reset();
    dim();
    move_cursor(row_off+22, center_col-2);
    printf("coords:  d2d4");
    move_cursor(row_off+23, center_col-2);
    printf("algebra: Nxd7");
    reset();

    // command list text
    move_cursor(row_off+21, center_col+17);
    dim();
    underline();
    printf("   commands   ");
    reset();
    dim();
    move_cursor(row_off+22, center_col+17);
    printf("/draw  /resign");
    move_cursor(row_off+23, center_col+17);
    printf("/color /sound");
    reset();

    // borders
    // only print borders if row_off is greater than 1
    if (row_off > 1 && s.ws_col > 90) {
        //top line
        move_cursor(row_off, center_col-41);
        printf("┏");
        for (int i=0; i<81; i++) {
            printf("━");
        }
        printf("┓");

        // bottom line
        move_cursor(row_off+25, center_col-41);
        printf("┗");
        for (int i=0; i<81; i++) {
            printf("━");
        }
        printf("┛");

        // left line
        for (int i=0; i<24; i++) {
            move_cursor(row_off+1+i, center_col-41);
            printf("┃");
        }

        // right line
        for (int i=0; i<24; i++) {
            move_cursor(row_off+1+i, center_col+41);
            printf("┃");
        }
    }
    
    print_input(row_off+21, center_col-30);
    move_cursor(row_off+23, center_col-29+input_i);

    fflush(0);
}

void draw_login_screen() {
    clear();

    move_cursor(2, center_col-22);
    printf(" __   __           __        ___  __   __  ");
    move_cursor(3, center_col-22);
    printf("/__` /__` |__|    /  ` |__| |__  /__` /__` ");
    move_cursor(4, center_col-22);
    printf(".__/ .__/ |  |    \\__, |  | |___ .__/ .__/ ");
                                           
    fflush(0);
}

void main() {
    struct termios ttystate, ttysave;
    fd_set readset;

    // get current attributes and save them
    tcgetattr(STDIN_FILENO, &ttystate);
    ttysave = ttystate;

    //turn off canonical mode and echo
    ttystate.c_lflag &= ~(ICANON | ECHO);
    //minimum of number input read.
    // ttystate.c_cc[VMIN] = 1;

    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

    // set stdin as nonblocking so we can just poll it when necessary
    fcntl(0, F_SETFL, O_NONBLOCK);

    s.ws_row = 0;
    s.ws_col = 0;

    g = new_game();

    while (1) {
        struct winsize size = window_size();

        if (size.ws_row != s.ws_row || size.ws_col != s.ws_col) {
            s.ws_row = size.ws_row;
            s.ws_col = size.ws_col;
            center_col = s.ws_col / 2;
            center_row = s.ws_row / 2;
            row_off = (s.ws_row - 24) / 3;
            draw_game_screen();
        }

        // handle input (up to 32 inputs)
        char buf[32] = {0};
        size_t n = read(0, buf, 32);

        if (n == -1)
            goto sleep;

        for (int i=0; i<n; i++) {
            char c = buf[i];
            if ((isalnum(c) || c=='/') && input_i < 20) {
                input[input_i] = c;
                input[input_i+1] = 0;
                input_i++;

                printf("%c", c);

                if (input_i == 20)
                    cursor_left();

            } else if (c == 127 && input_i > 0) {
                if (input_i != 20)
                    cursor_left();
                printf(" ");
                cursor_left();

                input_i--;
                input[input_i] = 0;
            } else if (c == 27 && i+1<n && buf[i+1] == 91 && i+2<n) {
                i+=2;
                c = buf[i];

                if (c == 'A') {
                    scroll++;
                } else {
                    scroll--;
                }
            } else if (c == 10) {
                // clear the input on screen
                while (input_i > 0) {
                    if (input_i != 20)
                        cursor_left();
                    printf(" ");
                    cursor_left();
                    input_i--;
                }

                // parse the coords
                if (strlen(input) == 4) {
                    int from_col = tolower(input[0]) - 'a';
                    int from_row = tolower(input[1]) - '0' - 1;
                    int to_col = tolower(input[2]) - 'a';
                    int to_row = tolower(input[3]) - '0' - 1;

                    if (from_row >= 0 &&  from_col >= 0 && to_row >= 0 && to_col >= 0) {
                        if (from_row < 8 && from_col < 8 && to_row < 8 && to_col < 8) {
                            from_row = 7 - from_row;
                            to_row = 7 - to_row;

                            int from = from_row*8+from_col;
                            int to = to_row*8+to_col;
                            Move out;
                            char* error = do_move(g->board, from, to, white, &out);
                            if (error == NULL) {
                                white = !white;
                                draw_game_screen();
                            } else {
                                print_warning(error);
                                move_cursor(row_off+23, center_col-29+input_i);
                            }
                        }
                    }
                }

                // reset input to nothing
                input[0] = 0;
            }
        }
        fflush(0);

        sleep:
        usleep(10000);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &ttysave);
}
