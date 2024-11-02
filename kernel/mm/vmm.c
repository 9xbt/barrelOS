#include <stdint.h>
#include <dev/pit.h>
#include <lib/printf.h>

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

extern void vmm_load_pd(uint32_t *);
extern void vmm_enable_paging();

void vmm_install() {
    for (int i = 0; i < 1024; i++)
        page_directory[i] = 0x00000002;
    for (int i = 0; i < 1024; i++)
        first_page_table[i] = (i * 0x1000) | 3;
    page_directory[0] = ((unsigned int)first_page_table) | 3;

    vmm_load_pd(page_directory);
    vmm_enable_paging();

    printf("[%5d.%04d] %s:%d: initialized VMM and enabled paging\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__);
}