int main();

void _start() {
    int exit_status = main();
    asm("int $0x84": :"a"(exit_status));
}
