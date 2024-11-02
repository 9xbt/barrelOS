#include <dev/pit.h>
#include <cpu/tables/idt.h>
#include <cpu/io.h>
#include <lib/printf.h>

size_t pit_ticks = 0;

void pit_handler(struct registers *r) {
    pit_ticks++;
}

void pit_install(void) {
    uint16_t div = (uint16_t)(1193180 / PIT_FREQ);

    outb(0x43, 0x36);
    outb(0x40, (uint8_t)div);
    outb(0x40, (uint8_t)(div >> 8));
    irq_register(0, pit_handler);

    printf("[%5d.%04d] %s:%d: initialized PIT at %dkhz\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, PIT_FREQ / 1000);
}

void pit_reinstall() {
    irq_register(0, pit_handler);
}

void pit_sleep(size_t ms) {
    size_t start_ticks = pit_ticks;
    size_t end_ticks = start_ticks + ms * 10;

    while (pit_ticks < end_ticks) {
        asm volatile ("hlt");
    }
}