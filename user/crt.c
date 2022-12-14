#include "../syscall.h"

int main();

int syscall(int call, int arg) {
    asm("int $0x84": "+a"(call) : "b"(arg));
    return call;
}

_Noreturn
void _exit(int exit_status) {
    syscall(SYS_exit, exit_status);
    __builtin_unreachable();
}

void _start() {
    _exit(main());
}
