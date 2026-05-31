#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1

#define PIC_EOI     0x20

/* Remap PIC IRQs from 0-15 to offset-offset+15 */
void pic_init(void);

/* Send end-of-interrupt signal */
void pic_send_eoi(uint8_t irq);

/* Mask/unmask specific IRQ line */
void pic_set_mask(uint8_t irq);
void pic_clear_mask(uint8_t irq);

#endif /* PIC_H */
