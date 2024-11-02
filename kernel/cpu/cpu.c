#include <cpuid.h>
#include <cpu/cpu.h>

bool cpu_check_apic() {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
        return edx & CPUID_FEAT_EDX_APIC;
    return false;
}