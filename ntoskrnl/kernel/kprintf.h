#ifndef KPRINTF_H
#define KPRINTF_H

/* Kernel printf — outputs to both VGA and serial (COM1) */
void kprintf(const char *fmt, ...);

#endif /* KPRINTF_H */
