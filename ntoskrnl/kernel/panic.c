#include "panic.h"
#include "bsod.h"
#include "../driver/serial/serial.h"

void kernel_panic(const char *msg) {
    /* Disable interrupts */
    __asm__ volatile("cli");

    /* Output to serial for debugging */
    serial_print(COM1, "\n*** KERNEL PANIC: ");
    serial_print(COM1, msg);
    serial_print(COM1, " ***\n");

    /* Display BSOD */
    bsod(msg);

    /* Halt forever */
    for (;;) {
        __asm__ volatile("hlt");
    }
}
