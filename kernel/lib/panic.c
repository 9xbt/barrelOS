#include <stdarg.h>
#include <lib/printf.h>

void panic(const char *fmt, ...) {
    printf("[%5d.%04d] kernel panic: ", 0, 0);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");

    asm volatile ("cli");
    for (;;) asm volatile ("hlt");
}