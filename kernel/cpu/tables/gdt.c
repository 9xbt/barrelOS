#include <stdint.h>
#include <cpu/tables/gdt.h>
#include <dev/pit.h>
#include <lib/printf.h>

struct gdt_entry gdt_entries[3];
struct gdtr gdt_descriptor;

extern void gdt_flush(void);

void gdt_install(void) {
    gdt_set_entry(0, 0x0000, 0x00000000, 0b00000000, 0b00000000);
    gdt_set_entry(1, 0xFFFF, 0x00000000, 0b10011011, 0b11001111);
    gdt_set_entry(2, 0xFFFF, 0x00000000, 0b10010011, 0b11001111);

    gdt_descriptor = (struct gdtr) {
        .size = sizeof(struct gdt_entry) * 3 - 1,
        .offset = (uint32_t)&gdt_entries
    };

    gdt_flush();
    printf("[%5d.%04d] %s:%d: initialized GDT at address 0x%x\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, (uint32_t)&gdt_descriptor);
}

void gdt_set_entry(uint8_t index, uint16_t limit, uint32_t base, uint8_t access, uint8_t gran) {
    gdt_entries[index].limit = limit;
    gdt_entries[index].base_low = base & 0xFFFF;
    gdt_entries[index].base_mid = (base >> 16) & 0xFF;
    gdt_entries[index].access = access;
    gdt_entries[index].gran = gran;
    gdt_entries[index].base_high = (base >> 24) & 0xFF;
}