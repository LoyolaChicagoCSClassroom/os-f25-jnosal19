#include <stdint.h>
#include "terminal.h"

static uint8_t term_attr = 0x07; // light gray on black
static uint16_t cursor_row = 0;
static uint16_t cursor_col = 0;

static inline uint16_t make_cell(char ch, uint8_t attr) {
    return ((uint16_t)attr << 8) | (uint8_t)ch;
}

static void scroll_if_needed(void) {
    if (cursor_row < VGA_ROWS) return;

    // shift rows up: rows 1..24 -> 0..23
    for (uint16_t r = 1; r < VGA_ROWS; r++) {
        for (uint16_t c = 0; c < VGA_COLS; c++) {
            VGA_MEM[(r-1)*VGA_COLS + c] = VGA_MEM[r*VGA_COLS + c];
        }
    }
    // clear last row
    for (uint16_t c = 0; c < VGA_COLS; c++) {
        VGA_MEM[(VGA_ROWS-1)*VGA_COLS + c] = make_cell(' ', term_attr);
    }
    cursor_row = VGA_ROWS - 1;
}

void terminal_clear(void) {
    for (uint16_t r = 0; r < VGA_ROWS; r++) {
        for (uint16_t c = 0; c < VGA_COLS; c++) {
            VGA_MEM[r*VGA_COLS + c] = make_cell(' ', term_attr);
        }
    }
    cursor_row = 0;
    cursor_col = 0;
}

void terminal_set_color(uint8_t attr) {
    term_attr = attr;
}


void putc(int data) {
    char ch = (char)(data & 0xFF);

    if (ch == '\n') { cursor_col = 0; cursor_row++; scroll_if_needed(); return; }
    if (ch == '\r') { cursor_col = 0; return; }
    if (ch == '\t') {
        // 4-space tabs
        uint16_t next = (cursor_col + 4) & ~(4 - 1);
        while (cursor_col < next) putc(' ');
        return;
    }

    VGA_MEM[cursor_row * VGA_COLS + cursor_col] = make_cell(ch, term_attr);
    cursor_col++;
    if (cursor_col >= VGA_COLS) {
        cursor_col = 0;
        cursor_row++;
        scroll_if_needed();
    }
}
