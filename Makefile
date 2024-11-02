# Toolchain
AS = nasm
CC = clang
LD = ld

# Automatically find sources
KERNEL_S_SOURCES = $(shell cd kernel && find -L * -type f -name '*.S')
KERNEL_C_SOURCES = $(shell cd kernel && find -L * -type f -name '*.c')

# Get object files
KERNEL_OBJS := $(addprefix bin/kernel/, $(KERNEL_S_SOURCES:.S=.S.o) $(KERNEL_C_SOURCES:.c=.c.o))

# Flags
ASFLAGS = -f elf32 -g -F dwarf
CCFLAGS = -m32 -std=gnu11 -ffreestanding -O0 -Wall -Wextra -nostdlib -I kernel -fno-stack-protector -Wno-unused-parameter -fno-stack-check -fno-lto -mno-mmx -mno-80387 -mno-sse -mno-sse2 -mno-red-zone
QEMUFLAGS = -debugcon stdio -m 256M -cdrom bin/$(IMAGE_NAME).iso -rtc base=localtime -boot d -drive file=bin/fs.hdd,format=raw,if=none,id=nvm -device nvme,serial=nvme-1,drive=nvm -audiodev pa,id=speaker -machine pcspk-audiodev=speaker
LDFLAGS = -m elf_i386 -Tkernel/linker.ld -z noexecstack

# Output image name
IMAGE_NAME = image

all: boot kernel fs iso

run: all
	@qemu-system-i386 $(QEMUFLAGS)

run-gdb: all
	@qemu-system-i386 $(QEMUFLAGS) -S -s

bin/kernel/%.c.o: kernel/%.c
	@echo " CC $<"
	@mkdir -p "$$(dirname $@)"
	@$(CC) $(CCFLAGS) -c $< -o $@

bin/kernel/%.S.o: kernel/%.S
	@echo " AS $<"
	@mkdir -p "$$(dirname $@)"
	@$(AS) $(ASFLAGS) -o $@ $<

kernel: $(KERNEL_OBJS)
	@echo " LD kernel/*"
	@$(LD) $(LDFLAGS) $^ -o bin/kernel.elf

iso:
	@grub-file --is-x86-multiboot ./bin/kernel.elf; \
	if [ $$? -eq 1 ]; then \
		echo " error: kernel.elf is not a valid multiboot file"; \
		exit 1; \
	fi
	@mkdir -p iso_root/boot/grub/
	@cp bin/kernel.elf iso_root/boot/kernel.elf
	@cp boot/grub.cfg iso_root/boot/grub/grub.cfg
	@grub-mkrescue -o bin/$(IMAGE_NAME).iso iso_root/ -quiet 2>&1 >/dev/null | grep -v libburnia | cat
	@rm -rf iso_root/

fs:
	@echo " FS bin/fs.hdd"
	@dd if=/dev/zero of=bin/fs.hdd bs=1M count=64 status=none
	@mkfs.fat bin/fs.hdd -F 16 2>&1 >/dev/null | grep -v mke2fs | cat

clean:
	@rm -f $(BOOT_OBJS) $(KERNEL_OBJS)
	@rm -rf bin