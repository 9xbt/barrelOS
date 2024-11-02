#ifndef __PIT_H
#define __PIT_H

#include <stddef.h>

#define PIT_FREQ 10000

extern size_t pit_ticks;

void pit_install(void);
void pit_reinstall();
void pit_sleep(size_t ms);

#endif