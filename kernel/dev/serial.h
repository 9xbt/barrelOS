#pragma once

#include <cpu/io.h>
#include <lib/printf.h>

#define QEMU 0xE9
#define COM1 0x3F8

void serial_init();
void serial_puts(char *str);