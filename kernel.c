#include "vga.h"

void _start() {
    vga_clear_screen();
    for (int i = 0; i < 24; i++) {
        vga_print_string("hello world\n");
    }
}
