; =============================================================================
; NKLegacy Kernel Entry Point
; Multiboot-compliant header + kernel bootstrap
; =============================================================================

bits 32

; ---------------------------------------------------------------------------
; Multiboot Header Constants
; ---------------------------------------------------------------------------
MBOOT_MAGIC     equ 0x1BADB002
MBOOT_FLAGS     equ 0x00000003  ; Align modules + provide memory map
MBOOT_CHECKSUM  equ -(MBOOT_MAGIC + MBOOT_FLAGS)

; ---------------------------------------------------------------------------
; Multiboot Header (must be within first 8KB of kernel image)
; ---------------------------------------------------------------------------
section .multiboot
align 4
    dd MBOOT_MAGIC
    dd MBOOT_FLAGS
    dd MBOOT_CHECKSUM

; ---------------------------------------------------------------------------
; Kernel Stack (16KB)
; ---------------------------------------------------------------------------
section .bss
align 16
stack_bottom:
    resb 16384          ; 16 KB stack
stack_top:

; ---------------------------------------------------------------------------
; Kernel Entry Point
; ---------------------------------------------------------------------------
section .text
global _start
extern kmain

_start:
    ; Set up the kernel stack
    mov esp, stack_top

    ; Push multiboot info pointer and magic number
    push ebx            ; Multiboot info structure pointer
    push eax            ; Multiboot magic number

    ; Call the C kernel main function
    call kmain

    ; If kmain returns, halt the CPU
.hang:
    cli
    hlt
    jmp .hang
