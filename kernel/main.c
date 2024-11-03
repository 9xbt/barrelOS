#include <stdint.h>
#include <stddef.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <acpi/acpi.h>
#include <cpu/tables/gdt.h>
#include <cpu/tables/idt.h>
#include <dev/pic.h>
#include <dev/serial.h>
#include <lib/alloc.h>
#include <lib/limine.h>
#include <lib/printf.h>
#include <lib/console.h>
#include <sys/version.h>

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0  
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

uint64_t hhdm_offset;

struct console_t console;
struct framebuffer_t fb;

void _main(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED
     || !framebuffer_request.response
     || framebuffer_request.response->framebuffer_count < 1) {
        for (;;) asm ("hlt");
    }

    hhdm_offset = hhdm_request.response->offset;
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    fb.addr = framebuffer->address;
    fb.width = framebuffer->width;
    fb.height = framebuffer->height;
    fb.pitch = framebuffer->pitch;
    console_init(&console, &fb);

    serial_init();
    gdt_install();
    idt_install();
    pmm_install();
    vmm_install();
    malloc_init();
    acpi_install();

    printf("\nWelcome to \033[96mbarrelOS\033[0m!\n%s %d.%d %s %s %s\n",
        __kernel_name, __kernel_version_major,__kernel_version_minor,
        __kernel_build_date, __kernel_build_time, __kernel_arch);

    for (;;) asm ("hlt");
}