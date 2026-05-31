#include "kprintf.h"
#include "../driver/vga/vga.h"
#include "../driver/serial/serial.h"
#include <stdarg.h>

/* Simple integer to string (decimal) */
static void print_int(int value) {
    if (value < 0) {
        vga_putchar('-');
        serial_putchar(COM1, '-');
        value = -value;
    }

    char buf[12];
    int i = 0;

    if (value == 0) {
        buf[i++] = '0';
    } else {
        while (value > 0) {
            buf[i++] = '0' + (value % 10);
            value /= 10;
        }
    }

    /* Print in reverse */
    while (i > 0) {
        i--;
        vga_putchar(buf[i]);
        serial_putchar(COM1, buf[i]);
    }
}

/* Print unsigned integer */
static void print_uint(unsigned int value) {
    char buf[12];
    int i = 0;

    if (value == 0) {
        buf[i++] = '0';
    } else {
        while (value > 0) {
            buf[i++] = '0' + (value % 10);
            value /= 10;
        }
    }

    while (i > 0) {
        i--;
        vga_putchar(buf[i]);
        serial_putchar(COM1, buf[i]);
    }
}

/* Print unsigned integer in hex */
static void print_hex(unsigned int value) {
    static const char hex_chars[] = "0123456789ABCDEF";
    char buf[9];
    int i = 0;

    if (value == 0) {
        buf[i++] = '0';
    } else {
        while (value > 0) {
            buf[i++] = hex_chars[value & 0xF];
            value >>= 4;
        }
    }

    vga_putchar('0');
    serial_putchar(COM1, '0');
    vga_putchar('x');
    serial_putchar(COM1, 'x');

    while (i > 0) {
        i--;
        vga_putchar(buf[i]);
        serial_putchar(COM1, buf[i]);
    }
}

static void kput(char c) {
    if (c == '\n') {
        serial_putchar(COM1, '\r');
    }
    vga_putchar(c);
    serial_putchar(COM1, c);
}

void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': {
                    const char *s = va_arg(args, const char *);
                    if (!s) s = "(null)";
                    while (*s) kput(*s++);
                    break;
                }
                case 'd': {
                    int val = va_arg(args, int);
                    print_int(val);
                    break;
                }
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    print_uint(val);
                    break;
                }
                case 'x':
                case 'p': {
                    unsigned int val = va_arg(args, unsigned int);
                    print_hex(val);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    kput(c);
                    break;
                }
                case '%':
                    kput('%');
                    break;
                default:
                    kput('%');
                    kput(*fmt);
                    break;
            }
        } else {
            kput(*fmt);
        }
        fmt++;
    }

    va_end(args);
}
