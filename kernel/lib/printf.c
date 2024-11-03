#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <cpu/io.h>
#include <dev/serial.h>
#include <lib/libc.h>
#include <lib/console.h>

extern struct console_t console;

void parse_num(char *s, int *ptr, uint32_t val, uint32_t base, int width, int zero_padding) {
    char buf[32];
    int i = 0;

    if (val == 0) {
        while (width-- > 1) s[(*ptr)++] = zero_padding ? '0' : ' ';
        s[(*ptr)++] = '0';
        return;
    }

    while (val) {
        buf[i++] = (val % base) < 10 ? (val % base) + '0' : (val % base) - 10 + 'A';
        val /= base;
    }

    int padding = width - i;
    while (padding-- > 0) s[(*ptr)++] = zero_padding ? '0' : ' ';

    while (i > 0) s[(*ptr)++] = buf[--i];
}

void parse_hex(char *s, int *ptr, uint64_t val, bool lx) {
    int i = lx ? 16 : 8;
    while (i-- > 0) {
        s[(*ptr)++] = "0123456789abcdef"[val >> (i * 4) & 0x0F];
    }
}

void parse_string(char *s, int *ptr, char *str) {
    if (!str) {
        memcpy(s + *ptr, "(null)", 6);
        *ptr += 6;
        return;
    }

    while (*str) {
        s[(*ptr)++] = *str++;
    }
}

int sprintf(char *s, const char *fmt, va_list args) {
    int ptr = 0;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            int width = 0;
            int zero_padding = 0;

            if (*fmt == '0') {
                zero_padding = 1;
                fmt++;
            }

            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }

            switch (*fmt) {
                case 'd':
                    parse_num(s, &ptr, va_arg(args, int), 10, width, zero_padding);
                    break;
                case 'x':
                    parse_hex(s, &ptr, va_arg(args, uint32_t), false);
                    break;
                case 'l':
                    fmt++;
                    switch (*fmt) {
                        case 'x':
                            parse_hex(s, &ptr, va_arg(args, uint64_t), true);
                            break;
                    }
                    break;
                case 's':
                    parse_string(s, &ptr, va_arg(args, char *));
                    break;
                case 'c':
                    s[ptr++] = (char)va_arg(args, int);
                    break;
            }
        } else {
            s[ptr++] = *fmt;
        }
        fmt++;
    }
    return ptr;
}

int vprintf(const char *fmt, va_list args) {
    char buf[1024] = {-1};
    int ret = sprintf(buf, fmt, args);
    
    console_write(&console, buf);
    serial_puts(buf);

    return ret;
}

int printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buf[1024] = {-1};
    int ret = sprintf(buf, fmt, args);

    console_write(&console, buf);
    serial_puts(buf);

    va_end(args);
    return ret;
}

void mubsan_log(const char* fmt, ...) {
    printf("[%5d.%04d] ", 0, 0);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    
    asm volatile ("cli");
    for (;;) asm volatile ("hlt");
}