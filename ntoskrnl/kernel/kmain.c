/*
 * NKLegacy Kernel
 * Windows NT-inspired kernel operating system
 *
 * kmain.c - Kernel main entry point
 */

#include "kprintf.h"
#include "panic.h"
#include "bsod.h"
#include "../arch/i386/gdt.h"
#include "../arch/i386/idt.h"
#include "../driver/vga/vga.h"
#include "../driver/serial/serial.h"
#include "../driver/pic/pic.h"
#include "../driver/pit/pit.h"
#include "../driver/keyboard/keyboard.h"
#include "../exe/cmd.h"
#include "../fs/nkfs/nkfs.h"
#include <nk/string.h>

#define NK_VERSION "0.1.0"
#define NK_CODENAME "Legacy"

void kmain(uint32_t magic, void *mboot_info) {
    (void)magic;
    (void)mboot_info;

    /* ---- Phase 1: Early Hardware Init ---- */

    /* Initialize serial port for early debug output */
    serial_init(COM1);
    serial_print(COM1, "NKLegacy: Serial debug initialized\n");

    /* Initialize VGA text mode */
    vga_set_color(0x0F);  /* White on black */
    vga_clear();

    /* ---- Phase 2: CPU Tables ---- */

    serial_print(COM1, "NKLegacy: Loading GDT...\n");
    gdt_init();

    serial_print(COM1, "NKLegacy: Loading IDT...\n");
    idt_init();

    /* ---- Phase 3: Interrupt Hardware ---- */

    serial_print(COM1, "NKLegacy: Initializing PIC...\n");
    pic_init();

    serial_print(COM1, "NKLegacy: Initializing PIT...\n");
    pit_init();

    serial_print(COM1, "NKLegacy: Initializing keyboard...\n");
    keyboard_init();

    /* Enable interrupts */
    __asm__ volatile("sti");
    serial_print(COM1, "NKLegacy: Interrupts enabled\n");

    /* ---- Phase 4: Display Boot Banner ---- */

    /* NT-style boot header */
    vga_set_color(0x0B);  /* Light cyan */
    vga_print("NKLegacy Kernel ");
    vga_set_color(0x0F);  /* White */
    vga_print("v" NK_VERSION);
    vga_print(" [Build: " __DATE__ " " __TIME__ "]\n");

    vga_set_color(0x08);  /* Dark gray */
    vga_print("Copyright (c) 2026 William Beauregard. MIT License.\n");
    vga_print("Windows NT-inspired kernel operating system.\n\n");

    /* Initialization status */
    vga_set_color(0x0A);  /* Light green */
    vga_print("  [OK] ");
    vga_set_color(0x07);
    vga_print("Global Descriptor Table\n");

    vga_set_color(0x0A);
    vga_print("  [OK] ");
    vga_set_color(0x07);
    vga_print("Interrupt Descriptor Table\n");

    vga_set_color(0x0A);
    vga_print("  [OK] ");
    vga_set_color(0x07);
    vga_print("Programmable Interrupt Controller (8259)\n");

    vga_set_color(0x0A);
    vga_print("  [OK] ");
    vga_set_color(0x07);
    vga_print("Programmable Interval Timer (1000 Hz)\n");

    vga_set_color(0x0A);
    vga_print("  [OK] ");
    vga_set_color(0x07);
    vga_print("PS/2 Keyboard Driver\n");

    vga_set_color(0x0A);
    vga_print("  [OK] ");
    vga_set_color(0x07);
    vga_print("Serial Debug (COM1 @ 115200)\n");

    vga_print("\n");
    vga_set_color(0x0F);
    kprintf("NKLegacy kernel initialized. %u ticks elapsed.\n\n", pit_get_ticks());

    vga_set_color(0x0E);  /* Yellow */
    vga_print("nklegacy> ");
    vga_set_color(0x0F);

    serial_print(COM1, "NKLegacy: Kernel initialization complete\n");

    nkfs_init();

    /* ---- Phase 5: Shell Loop ---- */
    cmd_run();
}