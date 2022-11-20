#include "keyboard.h"
#include "../cpu/isr.h"
#include "../console.h"
#include "port.h"

static const char sc_ascii[] = {
    '?', '?', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y',
    'U', 'I', 'O', 'P', '[', ']', '?', '?', 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' ',
};

static void interrupt_handler(registers_t *r) {
    uint8_t scancode = port_byte_in(0x60);
    if (scancode < sizeof(sc_ascii)) {
        char string[] = {sc_ascii[scancode], '\0'};
        printk(string);
    }
}

void init_keyboard() {
    register_interrupt_handler(IRQ1, interrupt_handler);
}
