#include "gdt.h"
#include <nk/string.h>

/* 5 GDT entries: null, kernel code, kernel data, user code, user data */
static struct gdt_entry gdt_entries[5];
static struct gdt_ptr   gdt_pointer;

/* External assembly function to load the GDT */
extern void gdt_flush(uint32_t gdt_ptr_addr);

static void gdt_set_entry(int idx, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t gran) {
    gdt_entries[idx].base_low    = (base & 0xFFFF);
    gdt_entries[idx].base_middle = (base >> 16) & 0xFF;
    gdt_entries[idx].base_high   = (base >> 24) & 0xFF;

    gdt_entries[idx].limit_low   = (limit & 0xFFFF);
    gdt_entries[idx].granularity  = ((limit >> 16) & 0x0F) | (gran & 0xF0);

    gdt_entries[idx].access      = access;
}

void gdt_init(void) {
    gdt_pointer.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdt_pointer.base  = (uint32_t)&gdt_entries;

    /* Null segment */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Kernel code segment: base=0, limit=4GB, exec/read, ring 0 */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Kernel data segment: base=0, limit=4GB, read/write, ring 0 */
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* User code segment: base=0, limit=4GB, exec/read, ring 3 */
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    /* User data segment: base=0, limit=4GB, read/write, ring 3 */
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    gdt_flush((uint32_t)&gdt_pointer);
}
