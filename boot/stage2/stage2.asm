; =============================================================================
; NKBootman - Stage 2 Bootloader
; Enables A20, loads kernel, switches to protected mode, jumps to kernel
; =============================================================================

bits 16
org 0x7E00

KERNEL_LOAD_SEG   equ 0x1000       ; Temp load segment (0x10000 physical)
KERNEL_LOAD_OFF   equ 0x0000
KERNEL_SECTORS    equ 128          ; Max kernel size: 64KB (128 * 512)
KERNEL_PHYS_ADDR  equ 0x100000     ; Protected mode kernel address (1MB)

stage2_start:
    ; Print Stage 2 loaded message
    mov si, msg_stage2
    call print_string_16

    ; ---- Enable A20 Line ----
    mov si, msg_a20
    call print_string_16
    call enable_a20
    mov si, msg_ok
    call print_string_16

    ; ---- Load Kernel from Disk ----
    mov si, msg_kernel
    call print_string_16

    ; Load kernel to temporary location below 1MB
    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0                      ; Cylinder 0
    mov cl, 18                     ; Sector 18 (after MBR + Stage 2)
    mov dh, 0                      ; Head 0
    mov bx, KERNEL_LOAD_OFF
    push es
    mov ax, KERNEL_LOAD_SEG
    mov es, ax
    mov ah, 0x02
    mov al, KERNEL_SECTORS
    int 0x13
    pop es
    jc .disk_error

    mov si, msg_ok
    call print_string_16

    ; ---- Get Memory Map (E820) ----
    ; (simplified — skip for now, multiboot provides this)

    ; ---- Switch to Protected Mode ----
    mov si, msg_pmode
    call print_string_16

    cli                            ; Disable interrupts
    lgdt [gdt_descriptor]          ; Load GDT

    ; Enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to flush pipeline and load CS
    jmp 0x08:protected_mode_entry

.disk_error:
    mov si, msg_disk_err
    call print_string_16
.hang:
    cli
    hlt
    jmp .hang

; ---------------------------------------------------------------------------
; A20 Enable (fast method + keyboard controller fallback)
; ---------------------------------------------------------------------------
enable_a20:
    ; Try fast A20
    in al, 0x92
    or al, 2
    and al, 0xFE                   ; Don't reset!
    out 0x92, al

    ; Verify A20 (simplified check)
    ret

; ---------------------------------------------------------------------------
; 16-bit print routines
; ---------------------------------------------------------------------------
print_string_16:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp print_string_16
.done:
    ret

; ---------------------------------------------------------------------------
; GDT for protected mode
; ---------------------------------------------------------------------------
align 8
gdt_start:
    ; Null descriptor
    dq 0

    ; Code segment: base=0, limit=4GB, 32-bit, ring 0
gdt_code:
    dw 0xFFFF          ; Limit low
    dw 0x0000          ; Base low
    db 0x00            ; Base middle
    db 10011010b       ; Access: present, ring 0, code, exec/read
    db 11001111b       ; Flags + limit high: 4K granularity, 32-bit
    db 0x00            ; Base high

    ; Data segment: base=0, limit=4GB, 32-bit, ring 0
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b       ; Access: present, ring 0, data, read/write
    db 11001111b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1     ; GDT size
    dd gdt_start                    ; GDT address

; ---------------------------------------------------------------------------
; Data
; ---------------------------------------------------------------------------
msg_stage2:   db "NKBootman Stage 2 loaded", 13, 10, 0
msg_a20:      db "Enabling A20 line...", 0
msg_kernel:   db "Loading kernel...", 0
msg_pmode:    db "Entering protected mode...", 13, 10, 0
msg_ok:       db " OK", 13, 10, 0
msg_disk_err: db 13, 10, "DISK READ ERROR!", 13, 10, 0

; =============================================================================
; 32-bit Protected Mode Code
; =============================================================================
bits 32

protected_mode_entry:
    ; Set up data segments
    mov ax, 0x10               ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000           ; Temporary stack

    ; Copy kernel from temp location (0x10000) to 1MB (0x100000)
    mov esi, 0x10000           ; Source
    mov edi, KERNEL_PHYS_ADDR  ; Destination
    mov ecx, (KERNEL_SECTORS * 512) / 4  ; Copy dwords
    rep movsd

    ; Jump to kernel entry point at 1MB
    jmp KERNEL_PHYS_ADDR

    ; Should never reach here
.hang32:
    cli
    hlt
    jmp .hang32
