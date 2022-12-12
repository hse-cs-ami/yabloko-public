#include "keyboard.h"
#include "../cpu/isr.h"
#include "../console.h"
#include "port.h"

static const char sc_ascii[] = {
    '?', '?', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '?', '?', 'q', 'w', 'e', 'r', 't', 'y',
    'u', 'i', 'o', 'p', '[', ']', '\n', '?', 'a', 's', 'd', 'f', 'g',
    'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' ',
};

static void interrupt_handler(registers_t *r) {
    uint8_t scancode = port_byte_in(0x60);
    if (scancode < sizeof(sc_ascii)) {
        char c = sc_ascii[scancode];
        if (kbd_buf_size < 1024) {
            kbd_buf[kbd_buf_size++] = c;
        }
        char string[] = {c, '\0'};
        printk(string);
    }
}

char* kbd_buf;
unsigned kbd_buf_size;

void init_keyboard() {
    kbd_buf = (char*)(1 << 20);

    register_interrupt_handler(IRQ1, interrupt_handler);
}
