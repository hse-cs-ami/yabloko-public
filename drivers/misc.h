#pragma once
#include "port.h"

__attribute__((noreturn))
static inline void qemu_shutdown() {
	port_word_out(0x604, 0x2000);
	while (1) {
		asm("hlt");
	}
	__builtin_unreachable();
}
