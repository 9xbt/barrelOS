#include <stdint.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <dev/pit.h>
#include <lib/libc.h>
#include <lib/printf.h>
#include <acpi/acpi.h>
#include <acpi/madt.h>

bool acpi_use_xsdt = false;
void *acpi_root_sdt;

__attribute__((no_sanitize("alignment")))
void *acpi_find_table(const char *signature) {
    printf("[%5d.%04d] %s:%d: searching for signature '%s'\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, signature);

    if (!acpi_use_xsdt) {
        struct acpi_rsdt *rsdt = (struct acpi_rsdt*)acpi_root_sdt;
        uint32_t entries = (rsdt->sdt.length - sizeof(rsdt->sdt)) / 4;

        for (uint32_t i = 0; i < entries; i++) {
            struct acpi_sdt *sdt = (struct acpi_sdt*)HIGHER_HALF(*((uint32_t*)rsdt->table + i));
            if (!memcmp(sdt->signature, signature, 4)) {
                printf("[%5d.%04d] %s:%d: found '%s' at address 0x%x\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, signature, (uint32_t)sdt);
                return (void*)sdt;
            }
        }

        printf("[%5d.%04d] %s:%d: couldn't find table %s\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, signature);
        return NULL;
    }
    
    /* use xsdt */
    struct acpi_xsdt *rsdt = (struct acpi_xsdt*)acpi_root_sdt;
    uint32_t entries = (rsdt->sdt.length - sizeof(rsdt->sdt)) / 8;
        
    for (uint32_t i = 0; i < entries; i++) {
        struct acpi_sdt *sdt = (struct acpi_sdt*)HIGHER_HALF(*((uint32_t*)rsdt->table + i));
        if (!memcmp(sdt->signature, signature, 4)) {
            printf("[%5d.%04d] %s:%d: found '%s' at address 0x%x\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, signature, (uint32_t)sdt);
            return (void*)sdt;
        }
    }

    printf("[%5d.%04d] %s:%d: couldn't find table %s\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, signature);
    return NULL;
}

void acpi_install() {
    struct acpi_rsdp *rsdp = NULL;

    for (uint16_t *addr = (uint16_t*)0x000E0000; addr < (uint16_t*)0x000FFFFF; addr += 16) {
        if (!strncmp((const char*)addr, "RSD PTR ", 8)) {
            rsdp = (struct acpi_rsdp *)addr;
            printf("[%5d.%04d] %s:%d: found RSDP at address 0x%x\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, addr);
            break;
        }
    }

    if (!rsdp) {
        printf("[%5d.%04d] %s:%d: couldn't find ACPI\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__);
        return;
    }

    if (rsdp->revision != 0) {
        /* use xsdt */
        acpi_use_xsdt = true;
        struct acpi_xsdp *xsdp = (struct acpi_xsdp*)rsdp;
        acpi_root_sdt = (struct acpi_xsdt*)HIGHER_HALF(xsdp->xsdt_addr);
    } else {
        acpi_root_sdt = (struct acpi_xsdt*)HIGHER_HALF(rsdp->rsdt_addr);
    }

    vmm_map((uintptr_t)PHYSICAL(acpi_root_sdt), (uintptr_t)acpi_root_sdt, PTE_PRESENT);
    
    madt_init();
}