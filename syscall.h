#pragma once

enum {
	T_SYSCALL = 0x84,
	SYS_exit = 0,
	SYS_greet = 1,
};

int syscall(int call, int arg);
