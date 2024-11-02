#ifndef __ACPI_H
#define __ACPI_H

#include <stdint.h>

struct acpi_rsdp {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr;
} __attribute__((packed));

typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr;

    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) acpi_xsdp;

typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_sdt;

typedef struct {
    acpi_sdt sdt;
    char table[];
} acpi_rsdt;

typedef struct {
    acpi_sdt sdt;
    char table[];
} acpi_xsdt;

extern void *acpi_root_sdt;

void  acpi_install();
void *acpi_find_table(const char *signature);

#endif