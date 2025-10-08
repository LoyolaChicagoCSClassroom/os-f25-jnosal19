#include <stdint.h>
#include "interrupt.h"
#include "terminal.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_ADDRESS 0xB8000

static void print_hex_byte(uint8_t b) {
    const char *hex = "0123456789ABCDEF";
    putc(hex[(b >> 4) & 0xF]);
    putc(hex[b & 0xF]);
}

// VGA text buffer starts at physical memory address 0xB8000.
// Each entry is a 16-bit value: high byte = color attributes, low byte = ASCII character.
static uint16_t *vga_buffer = (uint16_t *)VGA_ADDRESS;

// Tracks the current cursor position (0 = top-left corner).
static uint16_t cursor_pos = 0;

/**
 * putc - writes a single character to the VGA text buffer.
 * @c: character to write
 *
 * Handles newlines by moving to the start of the next line.
 * Characters are printed in light gray on black (attribute 0x07).
 * If the screen fills up, the cursor wraps back to the top.
 */
void putc(int c) {
    if (c == '\n') {
        // Move cursor to start of next line
        cursor_pos = (cursor_pos / VGA_WIDTH + 1) * VGA_WIDTH;
    } else {
        // Write character and increment cursor
        vga_buffer[cursor_pos++] = (0x07 << 8) | (uint8_t)c;
    }

    // Wrap around if end of screen is reached
    if (cursor_pos >= VGA_WIDTH * VGA_HEIGHT) {
        cursor_pos = 0;
    }
}

/**
 * printf - prints a null-terminated string to the VGA text buffer.
 * @str: pointer to the string to print
 */
void printf(const char *str) {
    while (*str) {
        putc(*str++);
    }
}

// External declarations
extern unsigned char keyboard_map[128];
extern void putc(int c);

/**
 * kernel_main - entry point for the kernel after bootloader handoff.
 *
 * Initializes system interrupts, enables the keyboard,
 * and prints startup messages to the VGA display.
 * The CPU then halts until an interrupt occurs.
 */

void kernel_main() {
    // Initialize interrupt controllers and descriptor tables
    remap_pic();
    load_gdt();
    init_idt();

    // Unmask keyboard interrupt line (IRQ1) — harmless to leave on even for polling
    IRQ_clear_mask(1);

    // Print boot messages
    printf("Keyboard Driver Initialized\n");
    printf("Polling for scancodes (port 0x60)...\n\n");

    // P O L L I N G   L O O P  (per deliverables)
    // Read PS/2 status at 0x64; if LSB set, read scancode at 0x60 and print it.
    while (1) {
        uint8_t status = inb(0x64);             // PS/2 status register
        if (status & 0x01) {                     // LSB (OS bit) means output buffer has data
            uint8_t sc = inb(0x60);              // read scancode
            print_hex_byte(sc);
            putc(' ');
        }
        // Optional: tiny pause to avoid pegging CPU (NOP or just continue)
        asm volatile ("nop");
    }
}
