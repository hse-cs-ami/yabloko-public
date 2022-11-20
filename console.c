#include "console.h"
#include "drivers/vga.h"

void printk(const char* msg) {
    vga_print_string(msg);
}

void panic(const char* msg) {
    printk("Kernel panic: ");
    printk(msg);
    while (1) {
        asm("hlt");
    }
}
