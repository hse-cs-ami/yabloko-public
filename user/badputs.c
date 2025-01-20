#include "../syscall.h"
#include <stdint.h>

int main() {
    const char* message = "I hope the kernel does not panic...\n";
    syscall(SYS_puts, (uint32_t)message);
    syscall(SYS_puts, 0x1bad1dea);
    return 0;
}
