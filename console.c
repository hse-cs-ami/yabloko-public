#include "console.h"
#include "drivers/vga.h"
#include "drivers/uart.h"

void printk(const char* msg) {
    vga_print_string(msg);
    for (; *msg; ++msg) {
        uartputc(*msg);
    }
}

void panic(const char* msg) {
    printk("\nKernel panic: ");
    printk(msg);
    asm("cli");
    while (1) {
        asm("hlt");
    }
}
