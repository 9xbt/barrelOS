#include <stdint.h>
#include <acpi/acpi.h>
#include <acpi/madt.h>
#include <lib/printf.h>

struct madt_ioapic *madt_ioapic_list[64];
struct madt_iso *madt_iso_list[64];
uint32_t madt_ioapics = 0;
uint32_t madt_isos = 0;

struct madt_lapic_addr *lapic_addr;

__attribute__((no_sanitize("alignment")))
void madt_init() {
    struct acpi_madt *madt = (struct acpi_madt*)acpi_find_table("APIC");

    uint32_t i = 0;
    while (i <= madt->length - sizeof(madt)) {
        struct madt_entry *entry = (struct madt_entry*)(madt->table + i);

        switch (entry->type) {
            case 1:
                madt_ioapic_list[madt_ioapics++] = (struct madt_ioapic*)entry;
                break;
            case 2:
                madt_iso_list[madt_isos++] = (struct madt_iso*)entry;
                break;
            case 5:
                lapic_addr = (struct madt_lapic_addr*)entry;
                break;
        }

        i += entry->length;
    }

    printf("[%5d.%04d] %s:%d: MADT located at address %x\n", 0, 0, __FILE__, __LINE__, (uint32_t)madt);
    printf("[%5d.%04d] %s:%d: found %d I/O APIC%s\n", 0, 0, __FILE__, __LINE__, madt_ioapics, madt_ioapics == 1 ? "" : "s");
    printf("[%5d.%04d] %s:%d: ISO count: %d\n", 0, 0, __FILE__, __LINE__, madt_isos);
}