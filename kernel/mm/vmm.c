#include <mm/pmm.h>
#include <mm/vmm.h>
#include <dev/pic.h>
#include <lib/libc.h>
#include <lib/limine.h>
#include <lib/printf.h>

extern symbol text_start_ld;
extern symbol text_end_ld;
extern symbol rodata_start_ld;
extern symbol rodata_end_ld;
extern symbol data_start_ld;
extern symbol data_end_ld;

__attribute__((used, section(".limine_requests")))
struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

pagemap *vmm_kernel_pm;

__attribute__((no_sanitize("undefined")))
void vmm_switch_pm(pagemap *pm) {
    asm volatile("mov %0, %%cr3" ::"r"((uint64_t)PHYSICAL(pm)) : "memory");
}

__attribute__((no_sanitize("undefined")))
void vmm_install() {
    vmm_kernel_pm = (pagemap *)HIGHER_HALF(pmm_alloc(1));
    memset(vmm_kernel_pm, 0, PAGE_SIZE);

    uintptr_t phys_base = kernel_address_request.response->physical_base;
    uintptr_t virt_base = kernel_address_request.response->virtual_base;

    uintptr_t text_start = ALIGN_DOWN((uintptr_t)text_start_ld, PAGE_SIZE);
    uintptr_t text_end = ALIGN_UP((uintptr_t)text_end_ld, PAGE_SIZE);
    uintptr_t rodata_start = ALIGN_DOWN((uintptr_t)rodata_start_ld, PAGE_SIZE);
    uintptr_t rodata_end = ALIGN_UP((uintptr_t)rodata_end_ld, PAGE_SIZE);
    uintptr_t data_start = ALIGN_DOWN((uintptr_t)data_start_ld, PAGE_SIZE);
    uintptr_t data_end = ALIGN_UP((uintptr_t)data_end_ld, PAGE_SIZE);

    for (uintptr_t text = text_start; text < text_end; text += PAGE_SIZE)
        vmm_map(vmm_kernel_pm, text, text - virt_base + phys_base, PTE_PRESENT);
    for (uintptr_t rodata = rodata_start; rodata < rodata_end; rodata += PAGE_SIZE)
        vmm_map(vmm_kernel_pm, rodata, rodata - virt_base + phys_base, PTE_PRESENT | PTE_NX);
    for (uintptr_t data = data_start; data < data_end; data += PAGE_SIZE)
        vmm_map(vmm_kernel_pm, data, data - virt_base + phys_base, PTE_PRESENT | PTE_WRITABLE | PTE_NX);
    for (uintptr_t addr = 0; addr < 0x100000000; addr += PAGE_SIZE) {
        vmm_map(vmm_kernel_pm, addr, addr, PTE_PRESENT | PTE_WRITABLE);
        vmm_map(vmm_kernel_pm, (uintptr_t)HIGHER_HALF(addr), addr, PTE_PRESENT | PTE_WRITABLE);
    }

    printf("[%5d.%04d] %s:%d: initialized VMM and enabled paging\n", 0, 0, __FILE__, __LINE__);
}

__attribute__((no_sanitize("undefined")))
uintptr_t *vmm_get_next_lvl(uintptr_t *lvl, uintptr_t entry, uint64_t flags, bool alloc) {
    if (lvl[entry] & PTE_PRESENT) return HIGHER_HALF(PTE_GET_ADDR(lvl[entry]));
    if (!alloc) return NULL;

    uintptr_t *pml = (uintptr_t*)HIGHER_HALF(pmm_alloc(1));
    memset(pml, 0, PAGE_SIZE);
    lvl[entry] = (uintptr_t)PHYSICAL(pml) | flags;
    return pml;
}

__attribute__((no_sanitize("undefined")))
void vmm_map(pagemap *pm, uintptr_t virt, uintptr_t phys, uint64_t flags) {
    uintptr_t pml4i = (virt >> 39) & 0x1ff;
    uintptr_t pml3i = (virt >> 30) & 0x1ff;
    uintptr_t pml2i = (virt >> 21) & 0x1ff;
    uintptr_t pml1i = (virt >> 12) & 0x1ff;

    uintptr_t *pml3 = vmm_get_next_lvl(pm, pml4i, PTE_PRESENT | PTE_WRITABLE, true);
    uintptr_t *pml2 = vmm_get_next_lvl(pml3, pml3i, PTE_PRESENT | PTE_WRITABLE, true);
    uintptr_t *pml1 = vmm_get_next_lvl(pml2, pml2i, PTE_PRESENT | PTE_WRITABLE, true);

    pml1[pml1i] = phys | flags;
}