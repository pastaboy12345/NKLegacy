#include "vga.h"
#include "../../arch/i386/io.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t *)0xB8000)

static size_t cursor_x = 0;
static size_t cursor_y = 0;
static uint8_t color = 0x0F;

static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
  return (uint16_t)c | ((uint16_t)color << 8);
}

void vga_update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_set_cursor(size_t x, size_t y) {
    cursor_x = x;
    cursor_y = y;
    vga_update_cursor();
}

void vga_set_color(uint8_t c) { color = c; }

void vga_clear(void) {
  for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
    VGA_MEMORY[i] = vga_entry(' ', color);
  }
  cursor_x = 0;
  cursor_y = 0;
  vga_update_cursor();
}

// scroll the screen up by one line
static void vga_scroll(void) {
  for (size_t y = 1; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      VGA_MEMORY[(y - 1) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
    }
  }

  // clear last row
  for (size_t x = 0; x < VGA_WIDTH; x++) {
    VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', color);
  }

  cursor_y = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
  if (c == '\n') {
    cursor_x = 0;
    cursor_y++;
  } else if (c == '\r') {
    cursor_x = 0;
  } else if (c == '\b') {
    if (cursor_x > 0) {
      cursor_x--;
      VGA_MEMORY[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(' ', color);
    }
  } else if (c == '\t') {
    cursor_x = (cursor_x + 8) & ~7;
    if (cursor_x >= VGA_WIDTH) {
      cursor_x = 0;
      cursor_y++;
    }
  } else {
    VGA_MEMORY[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, color);
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
      cursor_x = 0;
      cursor_y++;
    }
  }

  if (cursor_y >= VGA_HEIGHT) {
    vga_scroll();
  }
  vga_update_cursor();
}

void vga_print(const char *str) {
  while (*str) {
    vga_putchar(*str++);
  }
}