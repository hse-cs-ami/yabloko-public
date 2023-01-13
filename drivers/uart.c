#include "uart.h"
#include "port.h"

static int uart;

enum {
    COM1 = 0x3f8,
};

void uartinit() {
    // Turn off the FIFO
    port_byte_out(COM1+2, 0);

    // 9600 baud, 8 data bits, 1 stop bit, parity off.
    port_byte_out(COM1+3, 0x80);    // Unlock divisor
    port_byte_out(COM1+0, 115200/9600);
    port_byte_out(COM1+1, 0);
    port_byte_out(COM1+3, 0x03);    // Lock divisor, 8 data bits.
    port_byte_out(COM1+4, 0);
    port_byte_out(COM1+1, 0x01);    // Enable receive interrupts.

    // If status is 0xFF, no serial port.
    if(port_byte_in(COM1+5) == 0xFF)
        return;
    uart = 1;

    // Acknowledge pre-existing interrupt conditions;
    // enable interrupts.
    port_byte_in(COM1+2);
    port_byte_in(COM1+0);
}

void
uartputc(char c)
{
    int i;

    if (!uart)
        return;
    for (i = 0; i < 128 && !(port_byte_in(COM1+5) & 0x20); i++) {
        asm("pause");
    }
    port_byte_out(COM1+0, c);
}
