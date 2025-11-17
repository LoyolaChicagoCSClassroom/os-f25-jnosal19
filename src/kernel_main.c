#include <stdint.h>
#include "interrupt.h"
#include "terminal.h"
#include "page.h"
#include "rprintf.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

void test_page_allocator(void) {
    // Initialize the page frame allocator
    init_pfa_list();
    esp_printf(putc, "Page Frame Allocator initialized.\n");

    // Test 1: Print initial state
    esp_printf(putc, "\n=== Test 1: Initial State ===\n");
    print_pfa_state();

    // Test 2: Allocate 2 pages
    esp_printf(putc, "\n=== Test 2: Allocating 2 pages ===\n");
    struct ppage *alloc1 = allocate_physical_pages(2);
    if (alloc1) {
        esp_printf(putc, "Allocated 2 pages starting at %x\n", alloc1->physical_addr);
        print_pfa_state();
    }

    // Test 3: Allocate 3 more pages
    esp_printf(putc, "\n=== Test 3: Allocating 3 more pages ===\n");
    struct ppage *alloc2 = allocate_physical_pages(3);
    if (alloc2) {
        esp_printf(putc, "Allocated 3 pages starting at %x\n", alloc2->physical_addr);
        print_pfa_state();
    }

    // Test 4: Free the first allocation
    esp_printf(putc, "\n=== Test 4: Freeing first 2 pages ===\n");
    free_physical_pages(alloc1);
    print_pfa_state();

    // Test 5: Free the second allocation
    esp_printf(putc, "\n=== Test 5: Freeing next 3 pages ===\n");
    free_physical_pages(alloc2);
    print_pfa_state();

    esp_printf(putc, "\n=== All tests complete ===\n");
}

// tiny printf for banners
static void printf(const char *s) { 
    while (*s) putc((unsigned char)*s++); 
}

// Forward declaration
void init_keyboard(void);

void main() {
    // Disable interrupts immediately
    __asm__ __volatile__("cli");
    
    // Initialize the terminal
    terminal_clear();

    // Test the page frame allocator FIRST
    test_page_allocator();

    // Set up interrupt infrastructure (but don't enable yet)
    remap_pic();
    load_gdt();
    init_idt();

    // Enable paging BEFORE enabling interrupts
    enable_paging();
    
    // IMPORTANT: Small delay to let paging stabilize
    for (volatile int i = 0; i < 10000; i++);

    // Print banner AFTER paging is enabled
    printf("\nKeyboard Driver Initialized\n");
    printf("Interrupt-driven mode enabled.\n");

    // Initialize keyboard controller BEFORE enabling interrupts
    init_keyboard();
    printf("Keyboard controller initialized.\n");
    
    // Enable keyboard interrupts (IRQ 1) at the PIC
    IRQ_clear_mask(1);
    
    printf("Press keys to see scancodes:\n\n");
    
    // CRITICAL: Delay before enabling interrupts globally
    for (volatile int i = 0; i < 10000; i++);

    // Enable interrupts globally - LAST step
    __asm__ __volatile__("sti");

    // Main loop
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
