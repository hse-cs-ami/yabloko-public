#include "string.h"

int strncmp(const char* s1, const char* s2, size_t size) {
    while (size && *s1 && *s2 && *s1 == *s2) {
        size--;
        s1++;
        s2++;
    }
    if (!size) {
        return 0;
    }
    return (unsigned char)(*s1) - (unsigned char)(*s2);
}

void memset(void* b, char c, size_t len) {
    char* p = b;
    for (size_t i = 0; i < len; ++i) {
        p[i] = c;
    }
}
