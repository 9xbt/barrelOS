#ifndef __CPU_H
#define __CPU_H

#include <stdbool.h>

#define CPUID_FEAT_EDX_APIC (1 << 9)

bool cpu_check_apic();

#endif