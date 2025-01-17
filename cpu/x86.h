#pragma once

static inline void
stosl(void *addr, int data, int cnt)
{
    asm volatile("cld; rep stosl" : : "D"(addr), "c"(cnt), "a"(data) : "memory");
}

static inline void
stosb(void *addr, unsigned char data, int cnt)
{
    asm volatile("cld; rep stosb" : : "D"(addr), "c"(cnt), "a"(data) : "memory");
}
