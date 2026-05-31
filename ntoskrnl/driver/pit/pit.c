#include "pit.h"
#include "../../arch/i386/io.h"
#include "../../arch/i386/idt.h"

#define PIT_CHANNEL0  0x40
#define PIT_CMD       0x43

static volatile uint32_t tick_count = 0;

static void pit_callback(registers_t *regs) {
    (void)regs;
    tick_count++;
}

void pit_init(void) {
    /* Set PIT to ~1000 Hz (divisor = 1193) */
    uint16_t divisor = PIT_FREQ / 1000;

    /* Channel 0, access mode lo/hi, mode 3 (square wave), binary */
    outb(PIT_CMD, 0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    /* Register timer callback on IRQ0 (IDT entry 32) */
    idt_register_handler(32, pit_callback);
}

uint32_t pit_get_ticks(void) {
    return tick_count;
}

void pit_sleep_ms(uint32_t ms) {
    uint32_t target = tick_count + ms;
    while (tick_count < target) {
        __asm__ volatile("hlt");
    }
}
