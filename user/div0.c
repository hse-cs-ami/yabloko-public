int main(void) {
    volatile int x = 1, y = 0;
    x /= y;
    return x;
}
