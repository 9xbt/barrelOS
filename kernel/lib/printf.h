#ifndef __PRINTF_H
#define __PRINTF_H

#include <stdarg.h>
#include <stdbool.h>

extern bool aux_output;

int sprintf(char *s, const char *fmt, va_list args);
int vprintf(const char *fmt, va_list args);
int printf(const char *fmt, ...);

#endif