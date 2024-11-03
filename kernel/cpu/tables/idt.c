#include <cpu/tables/idt.h>
#include <lib/libc.h>
#include <lib/panic.h>
#include <lib/printf.h>

__attribute__((aligned(0x10)))
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
        idt_set_entry(i, (uint64_t)idt_int_table[i], 0x28, 0x8E);
    }

    idt_descriptor = (struct idtr) {
        .size = sizeof(struct idt_entry) * 256 - 1,
        .offset = (uint64_t)idt_entries
    };

    asm volatile ("lidt %0" :: "m"(idt_descriptor));
    printf("[%5d.%04d] %s:%d: initialized IDT at address 0x%lx\n", 0, 0, __FILE__, __LINE__, (uint64_t)&idt_descriptor);
}

void idt_set_entry(uint8_t vector, uint64_t base, uint16_t selector, uint8_t type) {
    idt_entries[vector].base_low  = base & 0xFFFF;
    idt_entries[vector].selector  = selector;
    idt_entries[vector].zero      = 0x00;
    idt_entries[vector].type      = type;
    idt_entries[vector].base_mid  = (base >> 16) & 0xFFFF;
    idt_entries[vector].base_high = (base >> 32) & 0xFFFFFFFF;
    idt_entries[vector].reserved  = 0x00000000;
}

void irq_register(uint8_t vector, void *handler) {
    //if (ioapic_enabled && vector <= 15)
    //    ioapic_redirect_irq(0, vector + 32, vector, false);
    irq_handlers[vector] = handler;
}

void irq_unregister(uint8_t vector) {
    irq_handlers[vector] = (void *)0;
}

void isr_handler(struct registers *r) {
    if (r->int_no == 0xff) {
        return;
    }

    asm volatile ("cli");
    printf("[%5d.%04d] %s:%d: %s\n", 0, 0, __FILE__, __LINE__, isr_errors[r->int_no]);
    for (;;) asm volatile ("hlt");
}

void irq_handler(struct registers *r) {
    void(*handler)(struct registers *);
    handler = irq_handlers[r->int_no - 32];

    if (handler != NULL)
        handler(r);
    
    //if (lapic_enabled) lapic_eoi();
    //else pic_eoi(r.int_no);
}