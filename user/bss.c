#include "../syscall.h"

static char zeroes[8000];

int main(void) {
    zeroes[7999] = 0;
    return 0;
}