; =============================================================================
; NKBootman - Stage 1 MBR Bootloader
; Loads Stage 2 from disk and jumps to it
; Must fit in exactly 512 bytes
; =============================================================================

bits 16
org 0x7C00

STAGE2_LOAD_ADDR equ 0x7E00     ; Load Stage 2 right after MBR
STAGE2_SECTORS   equ 16         ; Number of sectors to read for Stage 2

start:
    ; Set up segments and stack
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00              ; Stack grows downward from MBR

    ; Save boot drive number
    mov [boot_drive], dl

    ; Clear screen
    mov ah, 0x00
    mov al, 0x03                ; 80x25 text mode
    int 0x10

    ; Print boot banner
    mov si, msg_booting
    call print_string

    ; Load Stage 2 from disk
    mov si, msg_loading
    call print_string

    ; Disk read: INT 13h, AH=02h
    mov ah, 0x02                ; Read sectors
    mov al, STAGE2_SECTORS      ; Number of sectors
    mov ch, 0                   ; Cylinder 0
    mov cl, 2                   ; Sector 2 (1-indexed, sector 1 = MBR)
    mov dh, 0                   ; Head 0
    mov dl, [boot_drive]        ; Drive number
    mov bx, STAGE2_LOAD_ADDR    ; Buffer address ES:BX
    int 0x13
    jc disk_error

    ; Jump to Stage 2
    mov si, msg_ok
    call print_string

    mov dl, [boot_drive]        ; Pass boot drive to Stage 2
    jmp 0x0000:STAGE2_LOAD_ADDR

disk_error:
    mov si, msg_disk_err
    call print_string
.hang:
    cli
    hlt
    jmp .hang

; ---------------------------------------------------------------------------
; Subroutines
; ---------------------------------------------------------------------------
print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp print_string
.done:
    ret

; ---------------------------------------------------------------------------
; Data
; ---------------------------------------------------------------------------
boot_drive:   db 0
msg_booting:  db "NKBootman v1.0", 13, 10, 0
msg_loading:  db "Loading Stage 2...", 0
msg_ok:       db " OK", 13, 10, 0
msg_disk_err: db 13, 10, "DISK ERROR!", 13, 10, 0

; ---------------------------------------------------------------------------
; Padding and boot signature
; ---------------------------------------------------------------------------
times 510 - ($ - $$) db 0
dw 0xAA55
