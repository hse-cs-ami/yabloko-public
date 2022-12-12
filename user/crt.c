int main();

_Noreturn
void _exit(int exit_status) {
    asm("int $0x84": : "a"(0), "b"(exit_status));
    __builtin_unreachable();
}

void _start() {
    _exit(main());
}
