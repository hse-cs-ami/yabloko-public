#include "console.h"
#include "drivers/vga.h"
#include "drivers/uart.h"
#include "drivers/misc.h"

unsigned vga_get_cursor();
void vga_set_cursor(unsigned offset);
void vga_putc(char c);

void vga_print(const char* s) {
    while (*s) {
        vga_putc(*s++);
    }
}

void vga_print_times(const char* s, int n) {
    for (int i = 0; i < n; ++i) {
        vga_print(s);
    }
}

void uart_print(const char* s) {
    while (*s) {
        uartputc(*s++);
    }
}

void dump_screen(void) {
    uart_print("screen dump:\n");
    unsigned cursor = vga_get_cursor();
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            unsigned offset = i * COLS + j;
            uartputc(video_memory[2 * offset]);
            if (offset == cursor) {
                uartputc('#');
            }
        }
        uartputc('\n');
    }
}

void _start() {
    uartinit();

    vga_clear_screen();

    vga_set_cursor(0);
    vga_print_times("x", 78);
    dump_screen();

    vga_print("hello");
    dump_screen();

    vga_print_times("$", 77);
    dump_screen();

    vga_print_times("\n", 23);
    dump_screen();

    vga_print_times("x", 120);
    dump_screen();
    vga_print("\n");

    vga_print("=^._.^=\n");
    dump_screen();

    qemu_shutdown();
}
