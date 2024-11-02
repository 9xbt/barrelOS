#include <mm/pmm.h>
#include <mm/vmm.h>
#include <stdint.h>
#include <dev/pit.h>
#include <lib/panic.h>
#include <lib/printf.h>

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

extern symbol text_start_ld;
extern symbol text_end_ld;
extern symbol rodata_start_ld;
extern symbol rodata_end_ld;
extern symbol data_start_ld;
extern symbol data_end_ld;
extern symbol bss_start_ld;
extern symbol bss_end_ld;

extern void vmm_load_pd(uint32_t *);
extern void vmm_enable_paging();

/*
 * vmm_flush_tlb - flushes TLB cache for the specified virtual address
 */
void vmm_flush_tlb(uintptr_t virt) {
    __asm__ volatile ("invlpg (%0)" ::"r"(virt) : "memory");
}

/*
 * vmm_map - maps a page
 */
void vmm_map(uintptr_t phys, uintptr_t virt, uint32_t flags) {
    uint32_t pdindex = virt >> 22;
    uint32_t ptindex = (virt >> 12) & 0x03FF;

    /* check if the page directory entry is present */
    if (!(page_directory[pdindex] & 0x1)) {
        uint32_t *new_pt = (uint32_t*)pmm_alloc(1); /* allocate a new page table if it doesn't exist */

        for (int i = 0; i < 1024; i++)
            new_pt[i] = 0; /* clear page table entries*/
            
        page_directory[pdindex] = (uintptr_t)new_pt | PTE_PRESENT | PTE_WRITABLE; /* present, writable */
    }

    uint32_t *pt = (uint32_t*)(page_directory[pdindex] & ~0xFFF); /* get base address */

    pt[ptindex] = (phys & ~0xFFF) | flags; /* map the page */
    
    vmm_flush_tlb(virt); /* flush the tlb entry */
}

/*
 * vmm_unmap - unmaps a page
 */
void vmm_unmap(uintptr_t virt) {
    uint32_t pdindex = virt >> 22;
    uint32_t ptindex = (virt >> 12) & 0x03FF;

    /* check if the page directory entry is present */
    if (page_directory[pdindex] & 0x1) {
        uint32_t *pt = (uint32_t*)(page_directory[pdindex] & ~0xFFF); /* get base address */

        /* clear the page table entry */
        pt[ptindex] = 0; /* unmap the page */

        /* check if the page table is empty and free it */
        bool empty = true;
        for (int i = 0; i < 1024; i++) {
            if (pt[i] & 0x1) { /* check if any entry is present */
                empty = false;
                break;
            }
        }

        if (empty) {
            pmm_free(pt, 1); /* free the page table if it is empty */
            page_directory[pdindex] = 0x00000000; /* clear the pde*/
        }
    }

    vmm_flush_tlb(virt); /* flush the tlb entry */
}

/*
 * vmm_install - initializes the virtual memory manager
 */
void vmm_install() {
    for (int i = 0; i < 1024; i++)
        page_directory[i] = PTE_WRITABLE; /* read/write */
    for (int i = 0; i < 1024; i++)
        first_page_table[i] = (i * 0x1000) | PTE_PRESENT | PTE_WRITABLE; /* present, writable */
    page_directory[0] = ((uint32_t)first_page_table) | PTE_PRESENT | PTE_WRITABLE; /* present, writable */

    vmm_load_pd(page_directory);
    vmm_enable_paging();

    uintptr_t phys_base = (uintptr_t)text_start_ld;
    uintptr_t virt_base = 0xC0000000;

    for (uintptr_t text = (uintptr_t)text_start_ld; text < (uintptr_t)text_end_ld; text += PAGE_SIZE)
        vmm_map(text, text - virt_base + phys_base, PTE_PRESENT);
    for (uintptr_t rodata = (uintptr_t)rodata_start_ld; rodata < (uintptr_t)rodata_end_ld; rodata += PAGE_SIZE)
        vmm_map(rodata, rodata - virt_base + phys_base, PTE_PRESENT);
    for (uintptr_t data = (uintptr_t)data_start_ld; data < (uintptr_t)data_end_ld; data += PAGE_SIZE)
        vmm_map(data, data - virt_base + phys_base, PTE_PRESENT | PTE_WRITABLE);
    for (uintptr_t bss = (uintptr_t)bss_start_ld; bss < (uintptr_t)bss_end_ld; bss += PAGE_SIZE)
        vmm_map(bss, bss - virt_base + phys_base, PTE_PRESENT | PTE_WRITABLE);

    printf("[%5d.%04d] %s:%d: initialized VMM and enabled paging\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__);
}