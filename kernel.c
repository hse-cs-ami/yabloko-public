#include "console.h"
#include "drivers/vga.h"
#include "drivers/uart.h"
#include "drivers/keyboard.h"
#include "cpu/isr.h"


void _start() {
    uartinit();
    init_keyboard();
    load_idt();
    sti();

    vga_clear_screen();
    printk("\nYABLOKO\n> ");
    while (1) {
        asm("hlt");
    }
}
