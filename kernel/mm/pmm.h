#ifndef __PMM_H
#define __PMM_H

#define PAGE_SIZE 4096

#define DIV_CEILING(x, y) (x + (y - 1)) / y
#define ALIGN_UP(x, y) DIV_CEILING(x, y) * y
#define ALIGN_DOWN(x, y) (x / y) * y

#define HIGHER_HALF(ptr) ((void*)((uintptr_t)ptr) + hhdm_offset)
#define PHYSICAL(ptr) ((void*)((uintptr_t)ptr) - hhdm_offset)

#include <stdint.h>
#include <stddef.h>

extern uint64_t hhdm_offset;

void  pmm_install();
void *pmm_alloc(size_t page_count);
void  pmm_free(void *ptr, size_t page_count);

#endif