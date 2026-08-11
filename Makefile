UNAME := $(shell uname)

CC_EFI   = x86_64-w64-mingw32-g++

ifeq ($(UNAME), Darwin)
    CC_KERN  = x86_64-elf-g++
    LD_KERN  = x86_64-elf-ld
    OC_KERN  = x86_64-elf-objcopy
    DEBUGFS  = /opt/homebrew/opt/e2fsprogs/sbin/debugfs
	MKFS = /opt/homebrew/opt/e2fsprogs/sbin/mkfs.ext2 
else
    CC_KERN = g++
    LD_KERN = ld
    OC_KERN = objcopy
    DEBUGFS = debugfs
	MKFS = mkfs.ext2
endif

EFI_CFLAGS = -Ignu-efi/inc -Iinclude -Ignu-efi/inc/x86_64 -Ignu-efi/inc/protocol \
             -ffreestanding -fno-stack-protector -mno-red-zone -mno-avx\
             -mno-sse -DGNU_EFI_USE_MS_ABI -DCONFIG_x86_64

KERN_CFLAGS = -Iinclude -ffreestanding -fno-stack-protector -mno-red-zone -fno-threadsafe-statics \
              -mno-avx -mno-sse -fno-rtti -fno-exceptions -O2 -Wall

BIN_CFLAGS = -Iinclude -ffreestanding -fno-stack-protector -mno-red-zone \
              -mno-avx -mno-sse -fno-rtti -fno-exceptions -O2 -Wall

.PHONY: all run clean install-programs

all: build esp/EFI/BOOT/BOOTX64.EFI esp/kernel.bin

boot.o: src/boot.cpp
	$(CC_EFI) $(EFI_CFLAGS) -c $< -o $@

esp/EFI/BOOT/BOOTX64.EFI: boot.o | esp/EFI/BOOT
	$(CC_EFI) -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -o BOOTX64.EFI $< gnu-efi/x86_64/lib/libefi.a
	cp BOOTX64.EFI $@
	rm BOOTX64.EFI

esp/EFI/BOOT:
	mkdir -p esp/EFI/BOOT

build:
	mkdir -p build

hello.o: src/bin/hello.cpp
	$(CC_KERN) $(BIN_CFLAGS) -c $< -o build/hello.o

touch.o: src/bin/touch.cpp
	$(CC_KERN) $(BIN_CFLAGS) -c $< -o build/touch.o

ls.o: src/bin/ls.cpp
	$(CC_KERN) $(BIN_CFLAGS) -c $< -o build/ls.o

boottime.o: src/bin/boottime.cpp
	$(CC_KERN) $(BIN_CFLAGS) -c $< -o build/boottime.o

help.o: src/bin/help.cpp
	$(CC_KERN) $(BIN_CFLAGS) -c $< -o build/help.o

version.o: src/bin/version.cpp
	$(CC_KERN) $(BIN_CFLAGS) -c $< -o build/version.o

rm.o: src/bin/rm.cpp
	$(CC_KERN) $(BIN_CFLAGS) -c $< -o build/rm.o

kernel.o: src/kernel.cpp
	$(CC_KERN) $(KERN_CFLAGS) -c $< -o build/kernel.o

util.o: src/util.cpp
	$(CC_KERN) $(KERN_CFLAGS) -c $< -o build/util.o

isr.o: src/isr.s
	nasm -f elf64 $< -o build/isr.o

hello.elf: hello.o bin.ld
	$(LD_KERN) -T bin.ld -o $@ build/hello.o

touch.elf: touch.o bin.ld
	$(LD_KERN) -T bin.ld -o $@ build/touch.o

ls.elf: ls.o bin.ld
	$(LD_KERN) -T bin.ld -o $@ build/ls.o

boottime.elf: boottime.o bin.ld
	$(LD_KERN) -T bin.ld -o $@ build/boottime.o

help.elf: help.o bin.ld
	$(LD_KERN) -T bin.ld -o $@ build/help.o

version.elf: version.o bin.ld
	$(LD_KERN) -T bin.ld -o $@ build/version.o

rm.elf: rm.o bin.ld
	$(LD_KERN) -T bin.ld -o $@ build/rm.o

kernel.elf: kernel.o util.o isr.o font_psf.o kernel.ld
	$(LD_KERN) -T kernel.ld -o $@ build/kernel.o build/util.o build/isr.o build/font_psf.o

font_psf.o: fonts/default.psf
	$(OC_KERN) -O elf64-x86-64 -B i386 -I binary fonts/default.psf build/font_psf.o

kernel.bin: kernel.elf
	$(OC_KERN) -O binary $< $@

esp/kernel.bin: kernel.bin
	cp $< $@

disk.img:
	qemu-img create -f raw $@ 16M
	$(MKFS) -F -b 1024 disk.img

install-programs: touch.elf ls.elf boottime.elf help.elf version.elf rm.elf disk.img
	$(DEBUGFS) -w -R "mkdir /bin" disk.img 2>/dev/null || true
	$(DEBUGFS) -w -R "rm bin/touch.elf" disk.img 2>/dev/null || true
	$(DEBUGFS) -w -R "write touch.elf bin/touch.elf" disk.img
	$(DEBUGFS) -w -R "rm bin/ls.elf" disk.img 2>/dev/null || true
	$(DEBUGFS) -w -R "write ls.elf bin/ls.elf" disk.img
	$(DEBUGFS) -w -R "rm bin/boottime.elf" disk.img 2>/dev/null || true
	$(DEBUGFS) -w -R "write boottime.elf bin/boottime.elf" disk.img
	$(DEBUGFS) -w -R "rm bin/help.elf" disk.img 2>/dev/null || true
	$(DEBUGFS) -w -R "write help.elf bin/help.elf" disk.img
	$(DEBUGFS) -w -R "rm bin/version.elf" disk.img 2>/dev/null || true
	$(DEBUGFS) -w -R "write version.elf bin/version.elf" disk.img
	$(DEBUGFS) -w -R "rm bin/rm.elf" disk.img 2>/dev/null || true
	$(DEBUGFS) -w -R "write rm.elf bin/rm.elf" disk.img

run: disk.img install-programs
	qemu-system-x86_64 -m 4096 -bios RELEASEX64_OVMF.fd \
		-drive format=raw,file=fat:rw:esp,if=virtio \
		-drive format=raw,file=disk.img,if=ide,index=0 \
		-serial stdio

clean:
	rm -f *.o *.elf *.bin *.EFI
	rm -rf esp
	rm -rf build