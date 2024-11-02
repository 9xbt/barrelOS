#ifndef __HEAP_H
#define __HEAP_H

#include <stdint.h>
#include <stdbool.h>

#define HEAP_MAGIC 0xdeadbeef

struct heap_block {
    struct heap_block *next;
    struct heap_block *prev;
    uint32_t size; 
    uint32_t magic;
};

struct heap {
    struct heap_block *head;
};

struct heap *heap_create();
void *heap_alloc(struct heap *h, uint32_t n);
void heap_free(void *ptr);

#endif