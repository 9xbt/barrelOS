#include <mm/pmm.h>
#include <mm/vmm.h>
#include <cpu/cpu.h>
#include <dev/pic.h>
#include <dev/pit.h>
#include <dev/lapic.h>
#include <acpi/acpi.h>
#include <lib/printf.h>

bool lapic_enabled = false;
uint32_t lapic_ticks = 0;

__attribute__((no_sanitize("undefined")))
uint32_t lapic_read(uint32_t reg) {
    return *((uint32_t*)(HIGHER_HALF(LAPIC_REGS) + reg));
}

__attribute__((no_sanitize("undefined")))
void lapic_write(uint32_t reg, uint32_t value) {
    *((uint32_t*)(HIGHER_HALF(LAPIC_REGS) + reg)) = value;
}

void lapic_stop_timer() {
    lapic_write(LAPIC_TIMER_INITCNT, 0);
    lapic_write(LAPIC_TIMER_LVT, LAPIC_TIMER_DISABLE);
}

void lapic_oneshot(uint8_t vector, uint32_t ms) {
    lapic_stop_timer();

    lapic_write(LAPIC_TIMER_DIV, 0);
    lapic_write(LAPIC_TIMER_LVT, vector);
    lapic_write(LAPIC_TIMER_INITCNT, lapic_ticks * ms);
}

void lapic_calibrate_timer() {
    lapic_stop_timer();

    lapic_write(LAPIC_TIMER_DIV, 0);
    lapic_write(LAPIC_TIMER_LVT, (1 << 16) | 0xff);
    lapic_write(LAPIC_TIMER_INITCNT, 0xFFFFFFFF);

    pit_sleep(1); /* 1ms */

    lapic_write(LAPIC_TIMER_LVT, LAPIC_TIMER_DISABLE);

    uint32_t ticks = 0xFFFFFFFF - lapic_read(LAPIC_TIMER_CURCNT);
    lapic_ticks = ticks;

    lapic_stop_timer();

    printf("[%5d.%04d] %s:%d: calibrated Local APIC timer\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__);
}

void lapic_eoi() {
    lapic_write((uint8_t)LAPIC_EOI, 0);
}

void lapic_ipi(uint32_t id, uint8_t irq) {
    lapic_write(LAPIC_ICRHI, id << LAPIC_ICDESTSHIFT);
    lapic_write(LAPIC_ICRLO, irq);
}

void lapic_install() {
    if (!acpi_root_sdt || !cpu_check_apic()) {
        printf("[%5d.%04d] %s:%d: system does not have a Local APIC\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__);
        return;
    }

    pic_disable();
    vmm_map(LAPIC_REGS, (uintptr_t)HIGHER_HALF(LAPIC_REGS), PTE_PRESENT | PTE_WRITABLE);
    lapic_write(LAPIC_SIV, lapic_read(LAPIC_SIV) | 0x100);
    lapic_enabled = true;

    printf("[%5d.%04d] %s:%d: initialized Local APIC\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__);
}