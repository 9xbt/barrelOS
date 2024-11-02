#include <cpu/io.h>

void serial_puts(const char *str) {
    while (*str){
        outb(0xe9, *str++);
    }
}