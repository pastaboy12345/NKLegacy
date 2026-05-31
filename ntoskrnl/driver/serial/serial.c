#include "serial.h"
#include "../../arch/i386/io.h"

void serial_init(uint16_t port) {
    outb(port + 1, 0x00);    /* Disable interrupts */
    outb(port + 3, 0x80);    /* Enable DLAB (set baud rate divisor) */
    outb(port + 0, 0x01);    /* Divisor = 1 (115200 baud) lo byte */
    outb(port + 1, 0x00);    /*                            hi byte */
    outb(port + 3, 0x03);    /* 8 bits, no parity, one stop bit */
    outb(port + 2, 0xC7);    /* Enable FIFO, clear, 14-byte threshold */
    outb(port + 4, 0x0B);    /* IRQs enabled, RTS/DSR set */
}

static int serial_transmit_ready(uint16_t port) {
    return inb(port + 5) & 0x20;
}

void serial_putchar(uint16_t port, char c) {
    while (!serial_transmit_ready(port));
    outb(port, c);
}

void serial_print(uint16_t port, const char *str) {
    while (*str) {
        if (*str == '\n') {
            serial_putchar(port, '\r');
        }
        serial_putchar(port, *str++);
    }
}

static int serial_received(uint16_t port) {
    return inb(port + 5) & 1;
}

char serial_getchar(uint16_t port) {
    if (serial_received(port)) {
        return inb(port);
    }
    return 0;
}
