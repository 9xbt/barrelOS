#ifndef __LAPIC_H
#define __LAPIC_H

#define LAPIC_REGS          0xfee00000
#define LAPIC_EOI           0xb0
#define LAPIC_SIV           0xf0

#define LAPIC_TIMER_DIV     0x3E0
#define LAPIC_TIMER_LVT     0x320
#define LAPIC_TIMER_INITCNT 0x380
#define LAPIC_TIMER_DISABLE 0x10000
#define LAPIC_TIMER_CURCNT  0x390

#include <stdbool.h>

extern bool lapic_enabled;

void lapic_install();
void lapic_eoi();

#endif