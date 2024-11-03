#include <lib/libc.h>
#include <lib/console.h>

void fb_draw_char(struct framebuffer_t *fb, uint32_t x, uint32_t y, uint8_t c, uint32_t fore, uint32_t back) {
    for (int i = 0; i < 16; i++) {
        for (int o = 0; o < 8; o++) {
            if (builtin_font[c * 16 + i] & (1 << (7 - o))) {
                fb->addr[(y + i) * fb->width + x + o] = fore;
            } else {
                fb->addr[(y + i) * fb->width + x + o] = back;
            }
        }
    }
}

uint32_t ansi_to_rgb(int ansi) {
    uint32_t table[] = {
        0xFF000000, 0xFF0000AA, 0xFF00AA00, 0xFF00AAAA,
        0xFFAA0000, 0xFFAA00AA, 0xFFAA5500, 0xFFAAAAAA,
        0xFF555555, 0xFFFF5555, 0xFF55FF55, 0xFFFFFF55,
        0xFF5555FF, 0xFFFF55FF, 0xFF55FFFF, 0xFFFFFFFF
    };

    if (ansi == 0) {
        return 0xAAAAAA;
    } else if (ansi >= 30 && ansi <= 37) {
        return table[ansi - 30];
    } else if (ansi >= 40 && ansi <= 47) {
        return table[ansi - 40];
    } else if (ansi >= 90 && ansi <= 97) {
        return table[ansi - 82];
    } else if (ansi >= 100 && ansi <= 107) {
        return table[ansi - 92];
    } else {
        return 0x000000;
    }
}

void console_draw_cursor(struct console_t *console, int inverted) {
    for (int i = inverted ? 0 : 13; i < 16; i++) {
        for (int o = 0; o < 8; o++) {
            console->fb->addr[((console->y * 16) + i + console->border_y) *
                console->fb->width + (console->x * 8 + console->border_x) + o] =
                inverted ? ansi_to_rgb(console->background) : ansi_to_rgb(console->foreground);
        }
    }
}

void console_scroll(struct console_t *console) {
    memcpy((uint8_t *)console->fb->addr + (console->fb->pitch * console->border_y), (uint8_t *)console->fb->addr + ((16 + console->border_y) * console->fb->pitch), (console->fb->height - 16) * console->fb->pitch);
    memset((uint8_t *)console->fb->addr + (console->fb->height - 16 - console->border_y) * console->fb->pitch, 0x00, 16 * console->fb->pitch);

    console->y--;
}

void console_putchar(struct console_t *console, uint8_t c) {
    switch (c) {
        case '\n':
            console_draw_cursor(console, 1);
            console->x = console->width;
            break;
        case '\b':
            console_draw_cursor(console, 1);
            if (!console->x) {
                console->x = console->width - 1;
                console->y--;
            } else {
                console->x--;
            }
            console_draw_cursor(console, 1);
            break;
        default:
            fb_draw_char(console->fb, console->x * 8 + console->border_x, console->y * 16 +
                console->border_y, c, ansi_to_rgb(console->foreground), ansi_to_rgb(console->background));
            console->x++;
            break;
    }

    if (console->x >= console->width) { 
        console->x = 0;
        console->y++;

        if (console->y >= console->height) {
            console_scroll(console);
        }
    }
}

void console_write(struct console_t *console, char *str) {
    while (*str) {
        if (*str == '\033' && *(str+1) == '[') {
            str += 2;

            int num = 0;
            char buf[4] = {0};
            while (*str != 'm') {
                buf[num] = *str++;
                num++;
            }
            str += 1;

            int code = atoi(buf);
            if (code == 0) {
                console->foreground = 37;
                console->background = 40;
            } else if ((code >= 30 && code <= 37) || (code >= 90 && code <= 97)) {
                console->foreground = code;
            } else if ((code >= 40 && code <= 47) || (code >= 100 && code <= 107)) {
                console->background = code;
            }
        } else {
            console_putchar(console, (uint8_t)*str++);
        }
    }
    
    console_draw_cursor(console, 0);
}

void console_init(struct console_t *console, struct framebuffer_t *fb) {
    uint32_t border = fb->height % 16 / 2;

    console->width = (fb->width - border) / 8;
    console->height = fb->height / 16;
    console->border_x = border;
    console->border_y = border;
    console->fb = fb;
    console->foreground = 37;
    console->background = 40;
}