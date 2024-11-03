#include <stdint.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <lib/libc.h>
#include <lib/panic.h>
#include <lib/printf.h>
#include <lib/limine.h>
#include <acpi/acpi.h>
#include <acpi/madt.h>

__attribute__((used, section(".limine_requests")))
struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

bool acpi_use_xsdt = false;
void *acpi_root_sdt;

__attribute__((no_sanitize("alignment")))
void *acpi_find_table(const char *signature) {
    printf("[%5d.%04d] %s:%d: searching for signature '%s'\n", 0, 0, __FILE__, __LINE__, signature);

    if (!acpi_use_xsdt) {
        struct acpi_rsdt *rsdt = (struct acpi_rsdt*)acpi_root_sdt;
        uint32_t entries = (rsdt->sdt.length - sizeof(rsdt->sdt)) / 4;

        for (uint32_t i = 0; i < entries; i++) {
            struct acpi_sdt *sdt = (struct acpi_sdt*)HIGHER_HALF(*((uint32_t*)rsdt->table + i));
            if (!memcmp(sdt->signature, signature, 4)) {
                printf("[%5d.%04d] %s:%d: found '%s' at address 0x%x\n", 0, 0, __FILE__, __LINE__, signature, (uint32_t)sdt);
                return (void*)sdt;
            }
        }

        printf("[%5d.%04d] %s:%d: couldn't find table %s\n", 0, 0, __FILE__, __LINE__, signature);
        return NULL;
    }
    
    /* use xsdt */
    struct acpi_xsdt *rsdt = (struct acpi_xsdt*)acpi_root_sdt;
    uint32_t entries = (rsdt->sdt.length - sizeof(rsdt->sdt)) / 8;
        
    for (uint32_t i = 0; i < entries; i++) {
        struct acpi_sdt *sdt = (struct acpi_sdt*)HIGHER_HALF(*((uint32_t*)rsdt->table + i));
        if (!memcmp(sdt->signature, signature, 4)) {
            printf("[%5d.%04d] %s:%d: found '%s' at address 0x%x\n", 0, 0, __FILE__, __LINE__, signature, (uint32_t)sdt);
            return (void*)sdt;
        }
    }

    printf("[%5d.%04d] %s:%d: couldn't find table %s\n", 0, 0, __FILE__, __LINE__, signature);
    return NULL;
}

void acpi_install() {
    void *acpi_addr = rsdp_request.response->address;
    struct acpi_rsdp *rsdp = (struct acpi_rsdp *)acpi_addr;

    if (memcmp(rsdp->signature, "RSD PTR ", 8) || !rsdp->rsdt_addr) {
        panic("couldn't find ACPI");
    }
    return;

    if (rsdp->revision != 0) {
        /* use xsdt */
        acpi_use_xsdt = true;
        struct acpi_xsdp *xsdp = (struct acpi_xsdp*)rsdp;
        acpi_root_sdt = (struct acpi_xsdt*)HIGHER_HALF(xsdp->xsdt_addr);
    } else {
        acpi_root_sdt = (struct acpi_xsdt*)HIGHER_HALF(rsdp->rsdt_addr);
    }
    printf("[%5d.%04d] %s:%d: ACPI version %s\n", 0, 0, __FILE__, __LINE__, rsdp->revision == 0 ? "1.0" : "2.0+");
    
    madt_init();
}