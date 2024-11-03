#include <stddef.h>
#include <mm/pmm.h>
#include <lib/libc.h>
#include <lib/panic.h>
#include <lib/printf.h>
#include <lib/bitmap.h>
#include <lib/limine.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

extern uint64_t hhdm_offset;
struct limine_memmap_response* pmm_memmap = NULL;

uint8_t* pmm_bitmap = NULL;
uint64_t pmm_last_page = 0;
uint64_t pmm_used_pages = 0;
uint64_t pmm_page_count = 0;

void pmm_install() {
    pmm_memmap = memmap_request.response;
    struct limine_memmap_entry** entries = pmm_memmap->entries;
    struct limine_memmap_entry* default_memmap = NULL;

    uint64_t higher_address = 0;
    uint64_t top_address = 0;

    for (uint64_t i = 0; i < pmm_memmap->entry_count; i++) {
        if (entries[i]->type != LIMINE_MEMMAP_USABLE) continue;
        if (default_memmap == NULL || entries[i]->length > default_memmap->length) default_memmap = entries[i];
        
        top_address = entries[i]->base + entries[i]->length;
        if (top_address > higher_address) higher_address = top_address;
    }

    pmm_page_count = higher_address / PAGE_SIZE;
    uint64_t bitmap_size = ALIGN_UP(pmm_page_count / 8, PAGE_SIZE);

    pmm_bitmap = (uint8_t*)HIGHER_HALF(default_memmap->base);
    memset(pmm_bitmap, 0xFF, bitmap_size);
    default_memmap->base += bitmap_size;
    default_memmap->length -= bitmap_size;

    for (uint64_t i = 0; i < pmm_memmap->entry_count; i++) {
        if (entries[i]->type != LIMINE_MEMMAP_USABLE) continue;
        for (uint64_t o = 0; o < entries[i]->length; o += PAGE_SIZE)
            bitmap_clear(pmm_bitmap, (entries[i]->base + o) / PAGE_SIZE);
    }

    printf("[%5d.%04d] %s:%d: initialized PMM with bitmap size of %d\n", 0, 0, __FILE__, __LINE__, bitmap_size);
}

uint64_t pmm_find_pages(uint64_t page_count) {
    uint64_t pages = 0;
    uint64_t first_page = pmm_last_page;

    while (pages < page_count) {
        if (pmm_last_page == pmm_page_count) {
            panic("out of memory");
        }

        if (!bitmap_get(pmm_bitmap, pmm_last_page + pages)) {
            pages++;
            if (pages == page_count) {
                for (uint64_t i = 0; i < pages; i++) {
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
    uint64_t pages = pmm_find_pages(page_count);
    
    if (pages == 0) {
        pmm_last_page = 0;
        pages = pmm_find_pages(page_count);
    }

    uint64_t phys_addr = pages * PAGE_SIZE;

    return (void*)(phys_addr);
}

void pmm_free(void *ptr, size_t page_count) {
    uint64_t page = (uint64_t)ptr / PAGE_SIZE;

    for (uint64_t i = 0; i < page_count; i++)
        bitmap_clear(pmm_bitmap, page + i);
}