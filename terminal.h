#pragma once
#include <stdint.h>

void terminal_clear(void);
void terminal_set_color(uint8_t attr);
void putc(int data);  
