#include <cpu/tables/idt.h>
#include <dev/pic.h>
#include <dev/pit.h>
#include <dev/lapic.h>
#include <lib/libc.h>
#include <lib/panic.h>
#include <lib/printf.h>

struct idt_entry idt_entries[256];
struct idtr idt_descriptor;
extern void *idt_int_table[];

void *irq_handlers[256];

const char* isr_errors[32] = {
    "division by zero",
    "debug",
    "non-maskable interrupt",
    "breakpoint",
    "detected overflow",
    "out-of-bounds",
    "invalid opcode",
    "no coprocessor",
    "double fault",
    "coprocessor segment overrun",
    "bad TSS",
    "segment not present",
    "stack fault",
    "general protection fault",
    "page fault",
    "unknown interrupt",
    "coprocessor fault",
    "alignment check",
    "machine check",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved"
};

void idt_install(void) {
    for (uint16_t i = 0; i < 256; i++) {
        idt_set_entry(i, (uint32_t)idt_int_table[i], 0x08, 0x8E);
    }

    idt_descriptor = (struct idtr) {
        .size = sizeof(struct idt_entry) * 256 - 1,
        .offset = (uint32_t)idt_entries
    };

    asm volatile ("lidt %0" :: "m"(idt_descriptor));
    printf("[%5d.%04d] %s:%d: initialized IDT at address 0x%x\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, (uint32_t)&idt_descriptor);
}

void idt_set_entry(uint8_t index, uint32_t base, uint16_t selector, uint8_t type) {
    idt_entries[index].base_low = base & 0xFFFF;
    idt_entries[index].selector = selector;
    idt_entries[index].zero = 0x00;
    idt_entries[index].type = type;
    idt_entries[index].base_high = (base >> 16) & 0xFF;
}

void irq_register(uint8_t vector, void *handler) {
    irq_handlers[vector] = handler;
}

void irq_unregister(uint8_t vector) {
    irq_handlers[vector] = (void *)0;
}

void isr_handler(struct registers r) {
    if (r.int_no == 0xff) {
        return;
    }

    printf("[%5d.%04d] %s:%d: catched isr: %s\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, isr_errors[r.int_no]);

    if (r.int_no == 0x0e) {
        uint32_t cr2;
        asm volatile("mov %%cr2, %0" : "=r" (cr2));

        printf("[%5d.%04d] %s:%d: faulting address: 0x%x\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, cr2);
    }

    asm volatile ("cli");
    for (;;) asm volatile ("hlt");
}

void irq_handler(struct registers r) {
    void(*handler)(struct registers *);
    handler = irq_handlers[r.int_no - 32];

    if (handler != NULL)
        handler(&r);
    
    if (lapic_enabled) lapic_eoi();
    else pic_eoi(r.int_no);
}