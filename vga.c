#include "port.h"

char* const video_memory = (char*) 0xb8000;

enum {
    LINES = 25,
    COLS = 80,
    WHITE_ON_BLACK = 0x0f,

    VGA_CTRL_REGISTER = 0x3d4,
    VGA_DATA_REGISTER = 0x3d5,
    VGA_OFFSET_LOW = 0x0f,
    VGA_OFFSET_HIGH = 0x0e,
};

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
    video_memory[2 * offset + 1] = WHITE_ON_BLACK;
}

void vga_clear_screen() {
    for (unsigned i = 0; i < LINES * COLS; ++i) {
        vga_set_char(i, ' ');
    }
    vga_set_cursor(0);
}

void vga_print_string(const char* s) {
    int offset = vga_get_cursor();
    while (*s != 0) {
        vga_set_char(offset, *s);
        s++;
        offset++;
    }
    vga_set_cursor(offset);
}
