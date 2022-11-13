asm("jmp main");

#include "vga.h"

int main() {
    vga_clear_screen();
    for (int i = 0; i < 24; i++) {
        vga_print_string("hello world\n");
    }
    return 0;
}
