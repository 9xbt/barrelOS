#ifndef __GDT_H
#define __GDT_H

#include <stdint.h>

struct gdt_entry {
    uint16_t limit;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  gran;
    uint8_t  base_high;
} __attribute__((packed));

struct gdtr {
    uint16_t size;
    uint32_t offset;
} __attribute__((packed));

void gdt_install(void);
void gdt_set_entry(uint8_t index, uint16_t limit, uint32_t base, uint8_t access, uint8_t gran);

#endif