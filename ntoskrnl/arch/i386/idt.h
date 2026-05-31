#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* IDT entry structure */
struct idt_entry {
    uint16_t base_low;
    uint16_t sel;        /* Kernel segment selector */
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

/* IDT pointer structure for lidt instruction */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* Registers pushed by ISR stubs */
typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, useless_esp, ebx, edx, ecx, eax;  /* pusha */
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, esp, ss;  /* pushed by CPU */
} registers_t;

/* Interrupt handler callback type */
typedef void (*isr_handler_t)(registers_t *);

/* Initialize the Interrupt Descriptor Table */
void idt_init(void);

/* Register a handler for a specific interrupt number */
void idt_register_handler(uint8_t n, isr_handler_t handler);

#endif /* IDT_H */
