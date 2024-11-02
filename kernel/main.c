#include <stdint.h>
#include <cpu/tables/gdt.h>
#include <dev/vga.h>
#include <lib/libc.h>
#include <lib/printf.h>
#include <lib/multiboot.h>
#include <sys/version.h>

void _main(struct multiboot_info_t *mboot_info, uint32_t mboot_magic) {
    if (strstr((char *)mboot_info->cmdline, "--aux-output"))
        aux_output = true;

    gdt_install();

    printf("\nWelcome to \033[96mbarrelOS\033[0m!\n%s %d.%d %s %s %s\n",
        __kernel_name, __kernel_version_major,__kernel_version_minor,
        __kernel_build_date, __kernel_build_time, __kernel_arch);
}