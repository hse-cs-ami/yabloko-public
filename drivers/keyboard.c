#include "keyboard.h"
#include "../cpu/isr.h"
#include "../console.h"
#include "port.h"
#include "../lib/mem.h"

static const char sc_ascii[] = {
    '?', '?', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '?', '?', 'q', 'w', 'e', 'r', 't', 'y',
    'u', 'i', 'o', 'p', '[', ']', '\n', '?', 'a', 's', 'd', 'f', 'g',
    'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' ',
};

enum { kbd_buf_capacity = 1024 };

static void interrupt_handler(registers_t *r) {
    uint8_t scancode = port_byte_in(0x60);
    if (scancode < sizeof(sc_ascii)) {
        char c = sc_ascii[scancode];
        if (kbd_buf_size < kbd_buf_capacity) {
            kbd_buf[kbd_buf_size++] = c;
        }
        char string[] = {c, '\0'};
        printk(string);
    }
}

char* kbd_buf;
unsigned kbd_buf_size;

void init_keyboard() {
    kbd_buf = kmalloc(kbd_buf_capacity);

    register_interrupt_handler(IRQ1, interrupt_handler);
}
