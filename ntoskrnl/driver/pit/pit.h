#ifndef PIT_H
#define PIT_H

#include <stdint.h>

#define PIT_FREQ 1193182    /* Base oscillator frequency in Hz */

/* Initialize PIT to ~1000 Hz */
void pit_init(void);

/* Get current tick count since boot */
uint32_t pit_get_ticks(void);

/* Simple busy-wait sleep (in milliseconds) */
void pit_sleep_ms(uint32_t ms);

#endif /* PIT_H */
