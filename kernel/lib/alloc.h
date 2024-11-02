#ifndef __ALLOC_H
#define __ALLOC_H

#include <stddef.h>

void malloc_init();

void *kmalloc(size_t n);
void  kfree(void *ptr);

#endif