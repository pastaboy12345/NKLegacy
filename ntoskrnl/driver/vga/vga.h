#ifndef VGA_H
#define VGA_H

#include <stddef.h>
#include <stdint.h>

/* Initialize VGA text mode */
void vga_clear(void);
void vga_putchar(char c);
void vga_print(const char *str);
void vga_set_color(uint8_t c);

/* Hardware cursor control */
void vga_update_cursor(void);
void vga_set_cursor(size_t x, size_t y);

#endif /* VGA_H */