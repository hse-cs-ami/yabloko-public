#include "console.h"
#include "drivers/vga.h"

void printk(const char* msg) {
    vga_print_string(msg);
}
