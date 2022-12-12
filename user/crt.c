int main();

void _exit(int exit_status) {
    asm("int $0x84": : "a"(0), "b"(exit_status));
}

void _start() {
    _exit(main());
}
