#include <stdint.h>
#include "interrupt.h"
#include "terminal.h"   // brings in: int putc(int data);

// If you prefer, you can put this prototype in interrupt.h (see step B)
extern uint8_t inb(uint16_t port);

// tiny printf for banners
static void printf(const char *s) { while (*s) putc((unsigned char)*s++); }

static void print_hex_byte(uint8_t b) {
    static const char *hex = "0123456789ABCDEF";
    putc(hex[(b >> 4) & 0xF]);
    putc(hex[b & 0xF]);
}

void kernel_main() {
    remap_pic();
    load_gdt();
    init_idt();

    // harmless even while polling
    IRQ_clear_mask(1);

    printf("Keyboard Driver Initialized\n");
    printf("Polling for scancodes (port 0x60)...\n\n");

    while (1) {
        uint8_t status = inb(0x64);      // PS/2 status register
        if (status & 0x01) {             // output buffer has data
            uint8_t sc = inb(0x60);      // read scancode
            print_hex_byte(sc);
            putc(' ');
        }
        __asm__ __volatile__("nop");
    }
}
