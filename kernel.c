asm("jmp main");

#include "vga.h"

int main() {
    vga_clear_screen();
    vga_print_string("hello world");
    return 0;
}
