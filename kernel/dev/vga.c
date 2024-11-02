#include <stdint.h>
#include <cpu/io.h>
#include <dev/vga.h>
#include <lib/libc.h>

uint8_t vga_x = 0;
uint8_t vga_y = 0;
uint8_t vga_color = 0x07;
uint16_t *vga_buffer = (uint16_t *)0xB8000;

uint8_t ansi_to_vga(int ansi) {
    static uint8_t table[] = {
        0x00, 0x04, 0x02, 0x06, 0x01, 0x05, 0x03, 0x07,
        0x08, 0x0C, 0x0A, 0x0E, 0x09, 0x0D, 0x0B, 0x0F
    };

    if (ansi >= 30 && ansi <= 37)
        return table[ansi - 30];
    else if (ansi >= 40 && ansi <= 47)
        return table[ansi - 40];
    else if (ansi >= 90 && ansi <= 97)
        return table[ansi - 82];
    else if (ansi >= 100 && ansi <= 107)
        return table[ansi - 92];
    else return 0;
}

void vga_clear(void) {
    for (int i = 0; i < 80 * 25; i++)
        vga_buffer[i] = vga_color << 8;
}

void vga_puts(const char *str) {
    while (*str){
        if (*str == '\033' && *(str + 1) == '[') {
            str += 2;

            if (!strncmp(str, "2J", 2)) {
                vga_clear();
                str += 2;
                continue;
            }

            if (!strncmp(str, "H", 1)) {
                vga_x = 0;
                vga_y = 0;
                vga_update_cursor();
                str++;
                continue;
            }

            int num = 0;
            char buf[4] = {0};
            while (*str != 'm') {
                buf[num] = *str++;
                num++;
            }
            str++;

            int code = atoi(buf);
            if (code == 0) {
                vga_color = 0x07;
            } else if ((code >= 30 && code <= 37) || (code >= 90 && code <= 97)) {
                vga_color = (vga_color & 0xF0) | (ansi_to_vga(code) & 0x0F);
            } else if ((code >= 40 && code <= 47) || (code >= 100 && code <= 107)) {
                vga_color = (vga_color & 0x0F) | (ansi_to_vga(code) & 0xF0);
            }
        }
        vga_putchar(*str++);
    }
}

void vga_putchar(const char c) {
    switch (c) {
        case '\n':
            vga_x = 80;
            break;
        case '\b':
            if (vga_x == 0) {
                vga_x = 79;
                vga_y--;
            } else vga_x--;
            vga_buffer[vga_y * 80 + vga_x] = vga_color << 8;
            break;
        case '\t':
            vga_puts("    ");
            break;
        default:
            vga_buffer[vga_y * 80 + vga_x] = (vga_color << 8) | c;
            vga_x++;
            break;
    }

    if (vga_x >= 80) { 
        vga_x = 0;
        vga_y++;
    }

    if (vga_y >= 25)
        vga_scroll();

    vga_update_cursor();
}

void vga_scroll(void) {
    for (int i = 0; i < 80 * 24; i++)
        vga_buffer[i] = vga_buffer[i + 80];
    for (int i = 0; i < 80; i++)
        vga_buffer[80 * 24 + i] = vga_color << 8;
    vga_y--;
}

void vga_enable_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | 14);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
}

void vga_disable_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void vga_update_cursor(void) {
	uint16_t pos = vga_y * 80 + vga_x;
 
	outb(0x3D4, 0x0F);
	outb(0x3D5, pos & 0xFF);
	outb(0x3D4, 0x0E);
	outb(0x3D5, (pos >> 8) & 0xFF);
}