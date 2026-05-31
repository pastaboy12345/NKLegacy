#include "bsod.h"
#include "../driver/vga/vga.h"

void bsod(const char *message) {
    /* Classic NT blue screen: white on blue */
    vga_set_color(0x1F);
    vga_clear();

    vga_print("\n");
    vga_print("  *** STOP ***\n\n");
    vga_print("  A problem has been detected and NKLegacy has been shut down to prevent\n");
    vga_print("  damage to your computer.\n\n");

    if (message) {
        vga_print("  Error: ");
        vga_print(message);
        vga_print("\n\n");
    }

    vga_print("  If this is the first time you've seen this Stop error screen,\n");
    vga_print("  restart your computer. If this screen appears again, follow\n");
    vga_print("  these steps:\n\n");
    vga_print("  Check to make sure any new hardware or software is properly installed.\n");
    vga_print("  If problems continue, disable or remove any newly installed hardware\n");
    vga_print("  or software.\n\n");

    vga_print("  Technical Information:\n\n");
    vga_print("  *** KERNEL_PANIC\n\n");

    /* Halt */
    __asm__ volatile("cli");
    for (;;) {
        __asm__ volatile("hlt");
    }
}