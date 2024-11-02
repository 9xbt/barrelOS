#include <stdbool.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <lib/libc.h>
#include <lib/printf.h>
#include <dev/pit.h>

__attribute__((no_sanitize("undefined")))
struct heap *heap_create() {
    struct heap *h = (struct heap *)pmm_alloc(1);
    h->head = (struct heap_block *)pmm_alloc(1);
    h->head->next = h->head;
    h->head->prev = h->head;
    h->head->size = 0;
    h->head->magic = HEAP_MAGIC;

    printf("[%5d.%04d] %s:%d: created heap at address 0x%x\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, (uint32_t)h);
    return h;
}

__attribute__((no_sanitize("undefined")))
void *heap_alloc(struct heap *h, uint32_t n) {
    uint32_t pages = DIV_CEILING(sizeof(struct heap_block) + n, PAGE_SIZE);
    
    struct heap_block *block = (struct heap_block *)pmm_alloc(pages);
    vmm_map((uintptr_t)block, (uintptr_t)(block + 0xC0000000), PTE_PRESENT | PTE_WRITABLE);
    block->next = h->head;
    block->prev = h->head->prev;
    block->size = n;
    block->magic = HEAP_MAGIC;

    return (void*)block + sizeof(struct heap_block);
}

__attribute__((no_sanitize("undefined")))
void heap_free(void *ptr) {
    struct heap_block *block = (struct heap_block *)(ptr - sizeof(struct heap_block));

    if (block->magic != HEAP_MAGIC) {
        printf("[%5d.%04d] %s:%d: bad block magic at address 0x%x\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__, (uint32_t)block);
        return;
    }

    block->prev->next = block->next;
    block->next->prev = block->prev;
    uint32_t pages = DIV_CEILING(sizeof(struct heap_block) + block->size, PAGE_SIZE);

    pmm_free(block, pages);
    vmm_unmap((uintptr_t)(block + 0xC0000000));
}