#include <stdint.h>
#include "interrupt.h"
#include "terminal.h"

// tiny printf for banners
static void printf(const char *s) { 
    while (*s) putc((unsigned char)*s++); 
}

void kernel_main() {
    // Initialize the terminal
    terminal_clear();
    
    // Set up interrupt infrastructure
    remap_pic();      // Configure the Programmable Interrupt Controller
    load_gdt();       // Load the Global Descriptor Table
    init_idt();       // Initialize the Interrupt Descriptor Table
    
    // Enable keyboard interrupts (IRQ 1)
    IRQ_clear_mask(1);
    
    // Enable interrupts globally
    __asm__ __volatile__("sti");
    
    // Print banner
    printf("Keyboard Driver Initialized\n");
    printf("Interrupt-driven mode enabled.\n");
    printf("Press keys to see scancodes:\n\n");
    
    // Main loop - just wait for interrupts
    // The keyboard_handler will be called automatically when keys are pressed
    while (1) {
        __asm__ __volatile__("hlt");  // Halt until next interrupt
    }
}
