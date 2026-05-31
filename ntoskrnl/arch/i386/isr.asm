; =============================================================================
; ISR and IRQ Assembly Stubs
; Saves CPU state, calls C handler, restores state
; =============================================================================

bits 32

; ---------------------------------------------------------------------------
; ISR stubs for CPU exceptions (0-31)
; Some push error codes, some don't — we normalize by pushing a dummy 0
; ---------------------------------------------------------------------------

%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push dword 0            ; Dummy error code
    push dword %1           ; Interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    ; CPU already pushed error code
    push dword %1           ; Interrupt number
    jmp isr_common_stub
%endmacro

; Exceptions 0-31
ISR_NOERRCODE 0     ; Division By Zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non Maskable Interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Overflow
ISR_NOERRCODE 5     ; Bound Range Exceeded
ISR_NOERRCODE 6     ; Invalid Opcode
ISR_NOERRCODE 7     ; Device Not Available
ISR_ERRCODE   8     ; Double Fault
ISR_NOERRCODE 9     ; Coprocessor Segment Overrun
ISR_ERRCODE   10    ; Invalid TSS
ISR_ERRCODE   11    ; Segment Not Present
ISR_ERRCODE   12    ; Stack-Segment Fault
ISR_ERRCODE   13    ; General Protection Fault
ISR_ERRCODE   14    ; Page Fault
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; x87 Floating-Point Exception
ISR_ERRCODE   17    ; Alignment Check
ISR_NOERRCODE 18    ; Machine Check
ISR_NOERRCODE 19    ; SIMD Floating-Point Exception
ISR_NOERRCODE 20    ; Virtualization Exception
ISR_NOERRCODE 21    ; Reserved
ISR_NOERRCODE 22    ; Reserved
ISR_NOERRCODE 23    ; Reserved
ISR_NOERRCODE 24    ; Reserved
ISR_NOERRCODE 25    ; Reserved
ISR_NOERRCODE 26    ; Reserved
ISR_NOERRCODE 27    ; Reserved
ISR_NOERRCODE 28    ; Reserved
ISR_NOERRCODE 29    ; Reserved
ISR_NOERRCODE 30    ; Security Exception
ISR_NOERRCODE 31    ; Reserved

; ---------------------------------------------------------------------------
; IRQ stubs (hardware interrupts remapped to IDT 32-47)
; ---------------------------------------------------------------------------

%macro IRQ 2
global irq%1
irq%1:
    push dword 0            ; Dummy error code
    push dword %2           ; Interrupt number (32+)
    jmp irq_common_stub
%endmacro

IRQ 0,  32      ; PIT Timer
IRQ 1,  33      ; Keyboard
IRQ 2,  34      ; Cascade
IRQ 3,  35      ; COM2
IRQ 4,  36      ; COM1
IRQ 5,  37      ; LPT2
IRQ 6,  38      ; Floppy
IRQ 7,  39      ; LPT1 / Spurious
IRQ 8,  40      ; CMOS RTC
IRQ 9,  41      ; Free
IRQ 10, 42      ; Free
IRQ 11, 43      ; Free
IRQ 12, 44      ; PS/2 Mouse
IRQ 13, 45      ; FPU
IRQ 14, 46      ; Primary ATA
IRQ 15, 47      ; Secondary ATA

; ---------------------------------------------------------------------------
; Common ISR stub — saves state, calls C isr_handler, restores state
; ---------------------------------------------------------------------------
extern isr_handler

isr_common_stub:
    pusha               ; Push edi, esi, ebp, esp, ebx, edx, ecx, eax

    mov ax, ds
    push eax            ; Save data segment

    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; Pass pointer to registers_t
    call isr_handler
    add esp, 4

    pop eax             ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; Restore registers
    add esp, 8          ; Clean up int_no and err_code
    iret

; ---------------------------------------------------------------------------
; Common IRQ stub — saves state, calls C irq_handler, restores state
; ---------------------------------------------------------------------------
extern irq_handler

irq_common_stub:
    pusha

    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call irq_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    iret

; ---------------------------------------------------------------------------
; GDT/IDT flush routines
; ---------------------------------------------------------------------------
global gdt_flush
gdt_flush:
    mov eax, [esp + 4]     ; Get GDT pointer argument
    lgdt [eax]

    ; Reload segment registers with kernel data segment (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far jump to reload CS with kernel code segment (0x08)
    jmp 0x08:.flush_done
.flush_done:
    ret

global idt_flush
idt_flush:
    mov eax, [esp + 4]     ; Get IDT pointer argument
    lidt [eax]
    ret
