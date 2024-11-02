#include <mm/pmm.h>
#include <mm/vmm.h>
#include <stdint.h>
#include <acpi/acpi.h>
#include <cpu/tables/gdt.h>
#include <cpu/tables/idt.h>
#include <dev/vga.h>
#include <dev/pic.h>
#include <dev/pit.h>
#include <dev/lapic.h>
#include <dev/ioapic.h>
#include <lib/libc.h>
#include <lib/alloc.h>
#include <lib/printf.h>
#include <lib/multiboot.h>
#include <sys/version.h>

void _main(struct multiboot_info_t *mboot_info, uint32_t mboot_magic) {
    if (strstr((char *)mboot_info->cmdline, "--aux-output"))
        aux_output = true;

    gdt_install();
    idt_install();
    pic_install();
    pit_install();
    pmm_install(mboot_info);
    vmm_install();
    malloc_init();
    acpi_install();
    lapic_install();
    ioapic_install();

    printf("\nWelcome to \033[96mbarrelOS\033[0m!\n%s %d.%d %s %s %s\n",
        __kernel_name, __kernel_version_major,__kernel_version_minor,
        __kernel_build_date, __kernel_build_time, __kernel_arch);
}