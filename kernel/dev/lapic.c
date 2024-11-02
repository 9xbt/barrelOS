#include <mm/pmm.h>
#include <mm/vmm.h>
#include <cpu/cpu.h>
#include <dev/pic.h>
#include <dev/pit.h>
#include <dev/lapic.h>
#include <acpi/acpi.h>
#include <lib/printf.h>

__attribute__((no_sanitize("undefined")))
uint32_t lapic_read(uint32_t reg) {
    return *((uint32_t*)(HIGHER_HALF(LAPIC_REGS) + reg));
}

__attribute__((no_sanitize("undefined")))
void lapic_write(uint32_t reg, uint32_t value) {
    *((uint32_t*)(HIGHER_HALF(LAPIC_REGS) + reg)) = value;
}

void lapic_eoi() {
    lapic_write((uint8_t)LAPIC_EOI, 0);
}

void lapic_install() {
    if (!cpu_check_apic()) {
        printf("[%5d.%04d] %s:%d: system does not have a LAPIC\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__);
        return;
    }

    pic_disable();
    vmm_map(LAPIC_REGS, (uintptr_t)HIGHER_HALF(LAPIC_REGS), PTE_PRESENT | PTE_WRITABLE);
    lapic_write(LAPIC_SIV, lapic_read(LAPIC_SIV) | 0x100);

    printf("[%5d.%04d] %s:%d: initialized Local APIC\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__);
}