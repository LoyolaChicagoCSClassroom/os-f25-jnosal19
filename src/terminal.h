
#pragma once
#include <stdint.h>

// VGA constants
#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_MEM ((uint16_t*)0xB8000)

void terminal_clear(void);
void terminal_set_color(uint8_t attr);
int putc(int data);  // required signature
