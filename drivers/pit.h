#pragma once

typedef void (*timer_callback)(void);

void init_pit(void);
void add_timer_callback(timer_callback tc);

void msleep(int ms);
