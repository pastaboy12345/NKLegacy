#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

/* Initialize PS/2 keyboard driver */
void keyboard_init(void);

/* Get last pressed key (ASCII), returns 0 if no key available */
char keyboard_getchar(void);

#endif /* KEYBOARD_H */
