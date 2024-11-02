#ifndef __BITMAP_H
#define __BITMAP_H

#include <stdint.h>

void bitmap_set(uint8_t *bitmap, uint32_t bit) {
    bitmap[bit / 8] |= 1 << (bit % 8);
}

void bitmap_clear(uint8_t *bitmap, uint32_t bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

bool bitmap_get(uint8_t *bitmap, uint32_t bit) {
    return bitmap[bit / 8] & (1 << (bit % 8));
}

#endif