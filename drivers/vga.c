#include "port.h"
#include "../lib/string.h"
#include "cpu/memlayout.h"

static char* const video_memory = (char*) (KERNBASE + 0xb8000);

enum colors16 {
    black = 0,
    blue,
    green,
    cyan,
    red,
    magenta,
    brown,
    light_gray,
    dark_gray,
    light_blue,
    light_green,
    light_cyan,
    light_red,
    light_magenta,
    yellow,
    white,
};

static unsigned char get_color(unsigned char fg, unsigned char bg) {
    return (bg << 4) + fg;
}

enum {
    ROWS = 25,
    COLS = 80,

    VGA_CTRL_REGISTER = 0x3d4,
    VGA_DATA_REGISTER = 0x3d5,
    VGA_OFFSET_LOW = 0x0f,
    VGA_OFFSET_HIGH = 0x0e,
};

static unsigned get_offset(unsigned col, unsigned row) {
    return row * COLS + col;
}

static unsigned get_row_from_offset(unsigned offset) {
    return offset / COLS;
}

void vga_set_cursor(unsigned offset) {
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
    port_byte_out(VGA_DATA_REGISTER, (unsigned char) (offset >> 8));
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
    port_byte_out(VGA_DATA_REGISTER, (unsigned char) (offset & 0xff));
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

void vga_clear_screen() {
    for (unsigned i = 0; i < ROWS * COLS; ++i) {
        vga_set_char(i, ' ');
    }
    vga_set_cursor(0);
}

static unsigned scroll() {
    kmemmove(video_memory, video_memory + COLS, 2 * COLS * (ROWS-1));
    for (int col = 0; col < COLS; col++) {
        vga_set_char(get_offset(col, ROWS - 1), ' ');
    }
    return get_offset(0, ROWS - 1);
}

void vga_print_string(const char* s) {
    unsigned offset = vga_get_cursor();
    while (*s != 0) {
        if (*s == '\n') {
            offset = get_offset(0, get_row_from_offset(offset) + 1);
        } else {
            vga_set_char(offset, *s);
            offset++;
        }
        s++;
        if (offset > COLS * ROWS) {
            offset = scroll();
        }
    }
    vga_set_cursor(offset);
}
