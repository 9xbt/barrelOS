#include <stddef.h>
#include <mm/pmm.h>
#include <dev/pit.h>
#include <lib/libc.h>
#include <lib/panic.h>
#include <lib/printf.h>
#include <lib/bitmap.h>
#include <lib/multiboot.h>

extern void *end;

uint8_t *pmm_bitmap = NULL;
uint32_t pmm_last_page = 0;
uint32_t pmm_used_pages = 0;
uint32_t pmm_page_count = 0;
uint32_t pmm_usable_mem = 0;
uint32_t pmm_bitmap_size = 0;

void pmm_install(struct multiboot_info_t *mbd) {
    /* check bit 6 to see if we have a valid memory map */
    if(!(mbd->flags >> 6 & 0x1)) {
        panic("invalid multiboot memory map");
    }

    uint32_t higher_address = 0;
    uint32_t top_address = 0;
    struct multiboot_memory_map_t *kernel_mmmt = NULL;

    for(uint32_t i = 0; i < mbd->mmap_length;
        i += sizeof(struct multiboot_memory_map_t))
    {
        struct multiboot_memory_map_t* mmmt = 
            (struct multiboot_memory_map_t*) (mbd->mmap_addr + i);

        if (mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {
            if (mmmt->addr_low >= 0x100000 && kernel_mmmt == NULL) {
                kernel_mmmt = mmmt;
                pmm_usable_mem = kernel_mmmt->len_low;
            }

            top_address = mmmt->addr_low + mmmt->len_low;
            if (top_address > higher_address) {
                higher_address = top_address;
            }
        }
    }

    pmm_bitmap = (uint8_t *)&end;

    pmm_page_count = kernel_mmmt->len_low / PAGE_SIZE;
    pmm_bitmap_size = ALIGN_UP(pmm_page_count / 8, PAGE_SIZE);
    memset(pmm_bitmap, 0xFF, pmm_bitmap_size);

    kernel_mmmt->len_low -= (uint32_t)pmm_bitmap - kernel_mmmt->addr_low;
    kernel_mmmt->addr_low = (uint32_t)pmm_bitmap + pmm_bitmap_size;

    for(uint32_t i = 0; i < mbd->mmap_length;
        i += sizeof(struct multiboot_memory_map_t))
    {
        struct multiboot_memory_map_t* mmmt = 
            (struct multiboot_memory_map_t*) (mbd->mmap_addr + i);

        for (uint32_t o = 0; o < mmmt->len_low; o += PAGE_SIZE) {
            if (mmmt->type == MULTIBOOT_MEMORY_AVAILABLE && mmmt->addr_low >= 0x100000) {
                bitmap_clear(pmm_bitmap, (mmmt->addr_low + o) / PAGE_SIZE);
            }
        }
    }

    printf("[%5d.%04d] %s:%d: initialized PMM with bitmap size of %d\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, pmm_bitmap_size);
}

uint32_t pmm_find_pages(uint32_t page_count) {
    uint32_t pages = 0;
    uint32_t first_page = pmm_last_page;

    while (pages < page_count) {
        if (pmm_last_page == pmm_page_count) {
            panic("out of memory");
        }

        if (!bitmap_get(pmm_bitmap, pmm_last_page + pages)) {
            pages++;
            if (pages == page_count) {
                for (uint32_t i = 0; i < pages; i++) {
                    bitmap_set(pmm_bitmap, first_page + i);
                }

                pmm_last_page += pages;
                return first_page;
            }
        } else {
            pmm_last_page += (pages == 0 ? 1 : pages);
            first_page = pmm_last_page;
            pages = 0;
        }
    }
    return 0;
}

void *pmm_alloc(size_t page_count) {
    uint32_t pages = pmm_find_pages(page_count);
    
    if (pages == 0) {
        pmm_last_page = 0;
        pages = pmm_find_pages(page_count);
    }

    uint32_t phys_addr = pages * PAGE_SIZE;

    return (void*)(phys_addr);
}

void pmm_free(void *ptr, size_t page_count) {
    uint32_t page = (uint32_t)ptr / PAGE_SIZE;

    for (uint32_t i = 0; i < page_count; i++)
        bitmap_clear(pmm_bitmap, page + i);
}

uint32_t pmm_get_total_mem() {
    return pmm_bitmap_size * PAGE_SIZE * 8;
}

uint32_t pmm_get_usable_mem() {
    return pmm_usable_mem;
}

uint32_t pmm_get_used_mem() {
    uint32_t used_bytes = 0;

    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        if (pmm_bitmap[i] != 0) {
            for (uint8_t bit = 0; bit < 8; bit++) {
                if (pmm_bitmap[i] & (1 << bit)) {
                    used_bytes += PAGE_SIZE;
                }
            }
        }
    }

    return used_bytes;
}