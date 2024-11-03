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
ASFLAGS = -f elf64 -g -F dwarf
CCFLAGS = -Ikernel/ -g -O0 -pipe -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fPIE -m64 -march=x86-64 -mno-mmx -mno-sse -mno-sse2 -mno-80387 -mno-red-zone -fsanitize=undefined -MMD -MP
QEMUFLAGS = -debugcon stdio -m 256M -cdrom bin/$(IMAGE_NAME).iso -rtc base=localtime -boot d -display gtk,show-menubar=off,show-tabs=on
LDFLAGS = -m elf_x86_64 -nostdlib -static -pie --no-dynamic-linker -z text -z max-page-size=0x1000 -T kernel/linker.ld

# Output image name
IMAGE_NAME = image

all: limine kernel iso

run: all
	@qemu-system-x86_64 $(QEMUFLAGS)

run-gdb: all
	@qemu-system-x86_64 $(QEMUFLAGS) -S -s

bin/kernel/%.c.o: kernel/%.c
	@echo " CC $<"
	@mkdir -p "$$(dirname $@)"
	@$(CC) $(CCFLAGS) -c $< -o $@

bin/kernel/%.S.o: kernel/%.S
	@echo " AS $<"
	@mkdir -p "$$(dirname $@)"
	@$(AS) $(ASFLAGS) -o $@ $<

kernel: $(KERNEL_OBJS)
	@echo " LD "$^
	@$(LD) $(LDFLAGS) $^ -o bin/kernel.elf

iso:
	@rm -rf iso_root
	@mkdir -p iso_root/boot
	@cp -a bin/kernel.elf iso_root/boot/kernel.elf
	@mkdir -p iso_root/boot/limine
	@cp boot/limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	@mkdir -p iso_root/EFI/BOOT
	@cp limine/BOOTX64.EFI iso_root/EFI/BOOT/
	@cp limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	@xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin -quiet \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o bin/$(IMAGE_NAME).iso 2>&1 > /dev/null | grep -v xorriso | cat
	@./limine/limine bios-install bin/$(IMAGE_NAME).iso > /dev/null 2>&1
	@rm -rf iso_root

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v8.x-binary --depth=1
	make -C limine CC="cc" CFLAGS="-g -O2 -pipe" CPPFLAGS="" LDFLAGS="" LIBS=""
	cp limine/limine.h kernel/lib/limine.h

clean:
	@rm -f $(BOOT_OBJS) $(KERNEL_OBJS)
	@rm -rf bin