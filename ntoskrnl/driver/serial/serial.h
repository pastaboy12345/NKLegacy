#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

#define COM1 0x3F8
#define COM2 0x2F8

/* Initialize serial port for debug output */
void serial_init(uint16_t port);

/* Write a character to serial port */
void serial_putchar(uint16_t port, char c);

/* Write a string to serial port */
void serial_print(uint16_t port, const char *str);

/* Read a character from serial port */
char serial_getchar(uint16_t port);

#endif /* SERIAL_H */
