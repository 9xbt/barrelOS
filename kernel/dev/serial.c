#include <stdint.h>
#include <cpu/io.h>
#include <dev/serial.h>

uint16_t serial_port = 0;

void serial_init() {
    outb(COM1 + 1, 0);
    outb(COM1 + 3, 0x80);
    outb(COM1, 0x03);
    outb(COM1 + 1, 0);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
    outb(COM1 + 4, 0x1E);
    outb(COM1, 0xAE);

    if (inb(COM1) != 0xAE) {
        serial_port = QEMU;
    } else {
        serial_port = COM1;
        outb(COM1 + 4, 0x0F);
    }
    
    printf("[%5d.%04d] %s:%d: serial output on I/O port 0x%x\n", 0, 0, __FILE__, __LINE__, serial_port);
}

int serial_is_bus_empty() {
    return inb(COM1 + 5) & 0x20;
}

void serial_write_char(char c) {
    while (serial_is_bus_empty() == 0);
    outb(COM1, c);
}

void serial_puts(char *str) {
    while (*str) {
        char c = *str++;

        if (c == '\n') {
            outb(serial_port, '\r');
            outb(serial_port, '\n');
        } else {
            outb(serial_port, c);
        }
    }
}