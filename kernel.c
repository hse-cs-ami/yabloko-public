#include "console.h"
#include "drivers/vga.h"
#include "drivers/uart.h"
#include "drivers/misc.h"

void kprintf(const char* fmt, ...);

void _start() {
    uartinit();

    kprintf("\nyear: %u\n", 1984);
    kprintf("%u, %u, and %u\n", 0, 42, 31337);
    kprintf("0x%x\n", 0xdeadbeef);
    qemu_shutdown();
}
