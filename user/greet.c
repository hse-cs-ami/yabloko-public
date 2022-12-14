#include "../syscall.h"

int main(void) {
    syscall(SYS_greet, 0);
    syscall(SYS_greet, 0);
    return 0;
}
