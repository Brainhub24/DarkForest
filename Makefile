KERNEL_DIR := kernel
LIBC_DIR := libc
USERSPACE_DIR := userspace
COMMON_DIR := common

export ASM := nasm
export CC := i686-elf-g++
export LD = i686-elf-ld
# export CFLAGS := --std=c++17 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -mno-red-zone -fno-sized-deallocation  -fno-omit-frame-pointer -g -mno-omit-leaf-frame-pointer
export CFLAGS := --std=c++17 -ffreestanding -O2 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -mno-red-zone -fno-sized-deallocation -g -fno-omit-frame-pointer

export LDFLAGS := -ffreestanding -O2 -nostdlib

export CPP_FLAGS := 

.PHONY: all kernel userspace

all: kernel userspace fs

tests: clean set_tests_make_cmd all

set_tests_make_cmd:
	$(eval CPP_FLAGS := -DTESTS)

kernel:
	make -C $(KERNEL_DIR)/

userspace:
	make -C $(USERSPACE_DIR)/

fs:
	./fs.sh

clean:
	make -C $(KERNEL_DIR)/ clean
	make -C $(LIBC_DIR)/ clean
	make -C $(LIBC_DIR)/ clean
	make -C $(USERSPACE_DIR)/ clean