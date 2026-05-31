# NKLegacy

**NKLegacy** is a Windows NT-inspired kernel operating system for x86 (i386), written from scratch in C and x86 assembly.

## Features

- **NKBootman** — Custom two-stage bootloader (MBR → Protected Mode)
- **Multiboot** — Also boots via GRUB for easy testing
- **GDT / IDT** — Full protected-mode descriptor tables with ISR/IRQ handling
- **VGA Text Mode** — 80x25 console with colors and hardware cursor
- **Serial Debug** — COM1 output at 115200 baud for `qemu -serial stdio`
- **8259 PIC** — Remapped hardware interrupts
- **8253 PIT** — 1000 Hz system timer with tick counter
- **PS/2 Keyboard** — Scancode set 1 with shift support
- **kprintf** — Formatted kernel output to VGA + serial

## Building

### Prerequisites

- `gcc` (with `-m32` support)
- `nasm`
- `ld` (GNU linker)
- `grub-mkrescue` + `xorriso` (for ISO target)
- `qemu-system-i386` (for testing)

On Debian/Ubuntu:
```bash
sudo apt install gcc nasm grub-pc-bin grub-common xorriso qemu-system-x86
```

### Build & Run

```bash
# Build kernel + bootloader + disk image
make all

# Boot via GRUB ISO in QEMU (recommended)
make run

# Boot via NKBootman floppy image in QEMU
make run-bootman

# Clean build artifacts
make clean
```

## Project Structure

```
NKLegacy/
├── boot/                    # NKBootman bootloader
│   ├── stage1/mbr.asm       #   Stage 1: MBR (512 bytes)
│   └── stage2/stage2.asm    #   Stage 2: Protected mode setup
├── include/nk/              # Shared headers
│   ├── types.h
│   └── string.h
├── ntoskrnl/                # Kernel (ntoskrnl)
│   ├── arch/i386/           #   Architecture-specific
│   │   ├── boot.asm         #     Multiboot entry point
│   │   ├── gdt.c/h          #     Global Descriptor Table
│   │   ├── idt.c/h          #     Interrupt Descriptor Table
│   │   ├── isr.asm          #     ISR/IRQ assembly stubs
│   │   └── io.h             #     Port I/O helpers
│   ├── driver/              #   Hardware drivers
│   │   ├── vga/             #     VGA text mode
│   │   ├── serial/          #     COM1 serial debug
│   │   ├── pic/             #     8259 PIC
│   │   ├── pit/             #     8253/8254 PIT timer
│   │   └── keyboard/        #     PS/2 keyboard
│   ├── kernel/              #   Kernel core
│   │   ├── kmain.c          #     Entry point & init
│   │   ├── kprintf.c/h      #     Formatted output
│   │   ├── bsod.c/h         #     Blue Screen of Death
│   │   └── panic.c/h        #     Kernel panic handler
│   ├── lib/                 #   Runtime library
│   │   └── string.c         #     memset, memcpy, strlen, etc.
│   └── linker.ld            #   Kernel linker script
├── Makefile
└── LICENSE
```

## License

MIT License — see [LICENSE](LICENSE).