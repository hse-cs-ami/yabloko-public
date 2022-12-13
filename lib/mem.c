#include "mem.h"

static void* freeptr;

void* kmalloc(size_t size) {
    if (!freeptr) {
        freeptr = (void*)(1<<20);
    }
    void* result = freeptr;
    freeptr += size;
    return result;
}
