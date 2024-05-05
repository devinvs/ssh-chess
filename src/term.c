#include <stdio.h>
#include <stdbool.h>

void set_bg(int r, int g, int b) {
    printf("\x1b[48;2;%d;%d;%dm", r, g, b);
}

void bg_moves() {
    // printf("\x1b[1;40m");
    set_bg(50, 50, 50);
}

void fg_black() {
    printf("\x1b[30m");
}

void fg_red() {
    printf("\x1b[31m");
}

void fg_white() {
    printf("\x1b[38;2;255;255;255m");
}

void fg_gray() {
    printf("\x1b[38;2;180;180;180m");
}

void reset() {
    printf("\x1b[0m");
}

void bold() {
    printf("\x1b[1m");
}

void clear() {
    printf("\x1b[2J");
}

void underline() {
    printf("\x1b[4m");
}

void move_cursor(int row, int col) {
    printf("\x1b[%d;%dH", row, col);
}

void dim() {
    printf("\x1b[2m");
}

void cursor_left() {
    printf("\x1b[1D");
}
