#include <stdint.h>
#include "terminal.h"
#include "rprintf.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

// Multiboot header required for GRUB
const unsigned int multiboot_header[] __attribute__((section(".multiboot"))) = {
    MULTIBOOT2_HEADER_MAGIC,
    0,
    16,
    -(16 + MULTIBOOT2_HEADER_MAGIC),
    0,
    12
};

// Function to determine current execution level
static const char *get_execution_level(void) {
    // In kernel mode on i386, we're at privilege level 0
    return "CPL=0 (kernel mode)";
}

// Main kernel entry point
void main() {
    // Initialize terminal
    terminal_clear();
    terminal_set_color(0x07);  // Light gray on black

    // Print hello message using esp_printf from rprintf.c
    esp_printf(putc, "Hello from my kernel!\r\n");

    // Print current execution level (requirement #2)
    esp_printf(putc, "Current execution level: %s\r\n", get_execution_level());

    // Test basic functionality
    esp_printf(putc, "Testing putc function...\r\n");
    esp_printf(putc, "Character: %c, Number: %d, Hex: %x\r\n", 'A', 42, 255);

    // Test scrolling functionality (requirement #3)
    esp_printf(putc, "Testing scroll functionality:\r\n");
    for (int i = 0; i < 30; i++) {
        esp_printf(putc, "Line %d: This line should scroll when we exceed 25 rows\r\n", i);
    }

    // Keep kernel running
    while(1) {
        // Kernel main loop - in a real OS, this would handle scheduling, etc.
        __asm__ __volatile__("hlt");  // Halt until next interrupt
    }
}
