#ifndef __PMM_H
#define __PMM_H

#define PAGE_SIZE 4096

#define DIV_CEILING(x, y) (x + (y - 1)) / y
#define ALIGN_UP(x, y) DIV_CEILING(x, y) * y

#include <stddef.h>
#include <lib/multiboot.h>

void  pmm_install(struct multiboot_info_t *mbd);
void *pmm_alloc(size_t page_count);
void  pmm_free(void *ptr, size_t page_count);
uint32_t pmm_get_total_mem();
uint32_t pmm_get_usable_mem();
uint32_t pmm_get_used_mem();

#endif