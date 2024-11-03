#include <stdint.h>
#include <cpu/tables/gdt.h>
#include <lib/printf.h>

struct gdt_entry gdt_entries[9];
struct gdtr gdt_descriptor;

extern void gdt_flush(void);

void gdt_install(void) {
    gdt_set_entry(0, 0x0000, 0x00000000, 0x00, 0x00);
    gdt_set_entry(1, 0xffff, 0x00000000, 0x9a, 0x00);
    gdt_set_entry(2, 0xffff, 0x00000000, 0x93, 0x00);
    gdt_set_entry(3, 0xffff, 0x00000000, 0x9a, 0xcf);
    gdt_set_entry(4, 0xffff, 0x00000000, 0x93, 0xcf);
    gdt_set_entry(5, 0xffff, 0x00000000, 0x9b, 0xaf);
    gdt_set_entry(6, 0xffff, 0x00000000, 0x93, 0xaf);
    gdt_set_entry(7, 0xffff, 0x00000000, 0xfb, 0xaf);
    gdt_set_entry(8, 0xffff, 0x00000000, 0xf3, 0xaf);

    gdt_descriptor = (struct gdtr) {
        .size = sizeof(struct gdt_entry) * 9 - 1,
        .offset = (uint64_t)&gdt_entries
    };

    asm volatile ("lgdt %0" :: "m"(gdt_descriptor) : "memory");
    printf("[%5d.%04d] %s:%d: initialized GDT at address 0x%lx\n", 0, 0, __FILE__, __LINE__, (uint64_t)&gdt_descriptor);
}

void gdt_set_entry(uint8_t index, uint16_t limit, uint32_t base, uint8_t access, uint8_t gran) {
    gdt_entries[index].limit = limit;
    gdt_entries[index].base_low = base & 0xFFFF;
    gdt_entries[index].base_mid = (base >> 16) & 0xFF;
    gdt_entries[index].access = access;
    gdt_entries[index].gran = gran;
    gdt_entries[index].base_high = (base >> 24) & 0xFF;
}