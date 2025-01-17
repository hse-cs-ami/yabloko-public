#include "string.h"

void kmemmove(char* dst, char* src, size_t size) {
	if (dst == src) return;
	if (dst > src && dst < src + size) {  //   s d
		// copy right-to-left
		for (; size != 0; size--) {
			dst[size - 1] = src[size - 1];
		}
	} else {
		// copy left-to-right
		for (size_t i = 0; i < size; ++i) {
			dst[i] = src[i];
		}
	}
}

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
