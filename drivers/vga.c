#include "port.h"
#include "vga.h"

char* const video_memory = (char*) 0xb8000;

static unsigned char get_color(unsigned char fg, unsigned char bg) {
    return (bg << 4) + fg;
}

static unsigned get_offset(unsigned col, unsigned row) {
    return row * COLS + col;
}

static unsigned get_row_from_offset(unsigned offset) {
    return offset / COLS;
}

unsigned vga_get_cursor() {
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
    unsigned offset = port_byte_in(VGA_DATA_REGISTER) << 8;
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
    offset += port_byte_in(VGA_DATA_REGISTER);
    return offset;
}

void vga_set_char(unsigned offset, char c) {
    video_memory[2 * offset] = c;
    video_memory[2 * offset + 1] = get_color(light_gray, black);
}

static unsigned offset;

void vga_clear_screen() {
    for (unsigned i = 0; i < ROWS * COLS; ++i) {
        vga_set_char(i, ' ');
    }
}

void vga_print_string_noscroll(const char* s) {
    while (*s != 0) {
        if (*s == '\n') {
            offset = get_offset(0, get_row_from_offset(offset) + 1);
        } else {
            vga_set_char(offset, *s);
            offset++;
        }
        offset %= COLS * ROWS;
        s++;
    }
}
