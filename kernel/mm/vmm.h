#ifndef __VMM_H
#define __VMM_H

#include <stdint.h>

#define PTE_PRESENT 1ul
#define PTE_WRITABLE 2ul
#define PTE_USER 4ul

typedef char symbol[];

void vmm_install();
void vmm_map(uintptr_t phys, uintptr_t virt, uint32_t flags);
void vmm_unmap(uintptr_t virt);

#endif