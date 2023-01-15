#include "console.h"
#include "drivers/vga.h"
#include "drivers/uart.h"
#include "drivers/misc.h"

void show_vga_symbol_table(void);

void _start() {
    uartinit();

    show_vga_symbol_table();
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            unsigned offset = i * COLS + j;
            char c = video_memory[2 * offset];
            uartputc(c);
        }
        uartputc('\n');
    }
    qemu_shutdown();
}
