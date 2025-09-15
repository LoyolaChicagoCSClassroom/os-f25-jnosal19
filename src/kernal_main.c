#include "terminal.h"
#include "esp_printf.h"


static const char *exec_level(void) {
    return "CPL=0 (kernel mode)";
}

void kernel_main(void) {
    terminal_clear();
    terminal_set_color(0x07);

    esp_printf(putc, "Hello from my kernel!\r\n");
    esp_printf(putc, "Current execution level: %s\r\n", exec_level());

    // Force scrolling to prove it works:
    for (int i = 0; i < 30; i++) {
        esp_printf(putc, "Line %d: testing scroll...\r\n", i);
    }
}
