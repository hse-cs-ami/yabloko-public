#include "../syscall.h"
#include <stdint.h>

void userspace_puts(const char* s) {
    char c;
    while ((c = *s++)) {
        syscall(SYS_putc, c);
    }
}

int main() {
    const char* spell = "cra cra trif traf not sgnieflet\n";
    userspace_puts(spell);
    syscall(SYS_puts, (uint32_t)spell);
    return 0;
}
