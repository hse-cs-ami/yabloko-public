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
