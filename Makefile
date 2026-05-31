# ===========================================================================
# NKLegacy - Master Makefile
# Windows NT-inspired kernel operating system
# ===========================================================================

# --- Toolchain ---
CC      = gcc
AS      = nasm
LD      = ld
GRUB    = grub-mkrescue

# --- Compiler Flags ---
# GCC's built-in freestanding headers (stdint.h, stdarg.h, stddef.h)
GCC_INCDIR = $(shell $(CC) -m32 -print-file-name=include)

CFLAGS  = -m32 -ffreestanding -fno-builtin -fno-stack-protector \
          -nostdlib -nostdinc -Wall -Wextra -Werror \
          -isystem $(GCC_INCDIR) \
          -I$(ROOT)/include -I$(ROOT)/ntoskrnl \
          -g -O1
LDFLAGS = -m elf_i386 -T ntoskrnl/linker.ld --no-warn-rwx-segments
ASFLAGS = -f elf32

# --- Directories ---
ROOT       = .
BUILD      = build
BOOT_BUILD = $(BUILD)/boot
KERN_BUILD = $(BUILD)/kernel

# --- Source Files ---
# Assembly
ASM_SRCS = ntoskrnl/arch/i386/boot.asm \
           ntoskrnl/arch/i386/isr.asm

# C sources
C_SRCS   = ntoskrnl/kernel/kmain.c \
           ntoskrnl/kernel/bsod.c \
           ntoskrnl/kernel/kprintf.c \
           ntoskrnl/kernel/panic.c \
           ntoskrnl/arch/i386/gdt.c \
           ntoskrnl/arch/i386/idt.c \
           ntoskrnl/driver/vga/vga.c \
           ntoskrnl/driver/serial/serial.c \
           ntoskrnl/driver/pic/pic.c \
           ntoskrnl/driver/pit/pit.c \
           ntoskrnl/driver/keyboard/keyboard.c \
           ntoskrnl/lib/string.c \
           ntoskrnl/exe/pe.c \
           ntoskrnl/exe/nkbuild.c \
           ntoskrnl/exe/cmd.c \
           ntoskrnl/fs/nkfs/nkfs.c

# Object files
ASM_OBJS = $(patsubst %.asm,$(KERN_BUILD)/%.o,$(ASM_SRCS))
C_OBJS   = $(patsubst %.c,$(KERN_BUILD)/%.o,$(C_SRCS))
ALL_OBJS = $(ASM_OBJS) $(C_OBJS)

# --- Targets ---
.PHONY: all kernel bootloader image iso run run-serial clean

all: kernel bootloader image

# --- Kernel ---
kernel: $(BUILD)/nklegacy.elf

$(BUILD)/nklegacy.elf: $(ALL_OBJS) ntoskrnl/linker.ld
	@mkdir -p $(BUILD)
	$(LD) $(LDFLAGS) -o $@ $(ALL_OBJS)
	@echo "[LINK] $@"

# Compile C sources
$(KERN_BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "[CC]   $<"

# Assemble NASM sources
$(KERN_BUILD)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@
	@echo "[ASM]  $<"

# --- NKBootman Bootloader ---
bootloader: $(BOOT_BUILD)/mbr.bin $(BOOT_BUILD)/stage2.bin

$(BOOT_BUILD)/mbr.bin: boot/stage1/mbr.asm
	@mkdir -p $(BOOT_BUILD)
	$(AS) -f bin $< -o $@
	@echo "[BOOT] MBR: $@"

$(BOOT_BUILD)/stage2.bin: boot/stage2/stage2.asm
	@mkdir -p $(BOOT_BUILD)
	$(AS) -f bin $< -o $@
	@echo "[BOOT] Stage2: $@"

# --- Flat kernel binary for NKBootman ---
$(BUILD)/nklegacy.bin: $(BUILD)/nklegacy.elf
	objcopy -O binary $< $@
	@echo "[BIN]  $@"

# --- Disk Image (NKBootman) ---
image: $(BUILD)/nklegacy.img

$(BUILD)/nklegacy.img: $(BOOT_BUILD)/mbr.bin $(BOOT_BUILD)/stage2.bin $(BUILD)/nklegacy.bin
	@mkdir -p $(BUILD)
	# Create 1.44MB floppy image
	dd if=/dev/zero of=$@ bs=512 count=2880 2>/dev/null
	# Write MBR (sector 0)
	dd if=$(BOOT_BUILD)/mbr.bin of=$@ conv=notrunc bs=512 count=1 2>/dev/null
	# Write Stage 2 (sectors 1-17)
	dd if=$(BOOT_BUILD)/stage2.bin of=$@ conv=notrunc bs=512 seek=1 2>/dev/null
	# Write kernel (sector 17+)
	dd if=$(BUILD)/nklegacy.bin of=$@ conv=notrunc bs=512 seek=17 2>/dev/null
	@echo "[IMG]  $@ (1.44MB floppy)"

# --- GRUB ISO (alternative boot method) ---
iso: $(BUILD)/nklegacy.iso

$(BUILD)/nklegacy.iso: $(BUILD)/nklegacy.elf
	@mkdir -p $(BUILD)/iso/boot/grub
	cp $(BUILD)/nklegacy.elf $(BUILD)/iso/boot/nklegacy.elf
	@echo 'set timeout=0' > $(BUILD)/iso/boot/grub/grub.cfg
	@echo 'set default=0' >> $(BUILD)/iso/boot/grub/grub.cfg
	@echo '' >> $(BUILD)/iso/boot/grub/grub.cfg
	@echo 'menuentry "NKLegacy" {' >> $(BUILD)/iso/boot/grub/grub.cfg
	@echo '    multiboot /boot/nklegacy.elf' >> $(BUILD)/iso/boot/grub/grub.cfg
	@echo '    boot' >> $(BUILD)/iso/boot/grub/grub.cfg
	@echo '}' >> $(BUILD)/iso/boot/grub/grub.cfg
	$(GRUB) -o $@ $(BUILD)/iso 2>/dev/null
	@echo "[ISO]  $@"

# --- Run in QEMU ---
run: iso
	qemu-system-i386 -cdrom $(BUILD)/nklegacy.iso -serial stdio -m 32M

run-bootman: image
	qemu-system-i386 -fda $(BUILD)/nklegacy.img -serial stdio -m 32M -boot a

run-serial: iso
	qemu-system-i386 -cdrom $(BUILD)/nklegacy.iso -serial stdio -m 32M -display none -no-reboot

# --- Clean ---
clean:
	rm -rf $(BUILD)
	@echo "[CLEAN] Done"
