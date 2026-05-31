/* NKLegacy Kernel Linker Script */
/* Kernel is loaded at 1MB (0x100000) */

ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    _kernel_start = .;

    /* Multiboot header must be within first 8KB */
    .multiboot ALIGN(4) :
    {
        *(.multiboot)
    }

    .text ALIGN(4K) :
    {
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(4K) :
    {
        _bss_start = .;
        *(COMMON)
        *(.bss)
        *(.bss.*)
        _bss_end = .;
    }

    _kernel_end = .;

    /* Discard unnecessary sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.eh_frame)
    }
}
