BINARY_S := secure.elf
BINARY_LIB_S := cmse_import.o
BINARY_NS := nonsecure.elf
BINARY_ALL := image_s_ns.elf

MACHINE_NAME = mps2-an505

CMSIS ?= ../CMSIS_5
QEMU_PATH ?= ../qemu/arm-softmmu/qemu-system-arm
TOOLCHAIN ?= ../gcc-arm-none-eabi-8-2019-q3-update/bin
CROSS_COMPILE = $(TOOLCHAIN)/arm-none-eabi-

#TOOLCHAIN = ../gcc-arm-aarch64/bin
#CROSS_COMPILE = $(TOOLCHAIN)/aarch64-none-elf-

#TOOLCHAIN = ../gcc-linaro-7.3.1-2018.04-rc1-x86_64_armv8l-linux-gnueabihf/bin
#CROSS_COMPILE = $(TOOLCHAIN)/armv8l-linux-gnueabihf-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
GDB = $(CROSS_COMPILE)gdb
OBJ = $(CROSS_COMPILE)objdump
NM = $(CROSS_COMPILE)nm

TZ_VTOR_TABLE_ADDR = 0x00000000
LINKER_SCRIPT = gcc_arm.ld
LINKER_SCRIPT_NS = gcc_arm_ns.ld

SRC_ASM = $(CMSIS)/Device/ARM/ARMCM33/Source/GCC/startup_ARMCM33.S
SRC_C = $(CMSIS)/Device/ARM/ARMCM33/Source/system_ARMCM33.c \
	main.c \
	logPrint.c

INCLUDE_FLAGS = -I$(CMSIS)/Device/ARM/ARMCM33/Include \
	-I$(CMSIS)/CMSIS/Core/Include \
	-I.

COMMON_CFLAGS = -mcpu=cortex-m33 \
  -g \
  $(INCLUDE_FLAGS) \
  -nostartfiles -ffreestanding \
  -mthumb

CFLAGS = \
  $(COMMON_CFLAGS) \
  -DARMCM33_DSP_FP_TZ \
  -mcmse \
  -DTZ_VTOR_TABLE_ADDR=$(TZ_VTOR_TABLE_ADDR)


#	--specs=nosys.specs \
#	-Wall \

SECURE_LINKER_ARGS = \
  -Xlinker --sort-section=alignment \
  -Xlinker --cmse-implib \
  -Xlinker --out-implib=$(BINARY_LIB_S)

OBJS = main.o logPrint.o $(CMSIS)/Device/ARM/ARMCM33/Source/system_ARMCM33.o
OBJS_NS = main_ns.o $(CMSIS)/Device/ARM/ARMCM33/Source/system_ARMCM33.o

all: $(BINARY_S) $(BINARY_NS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

boot.o: $(SRC_ASM)
	$(CC) $(CFLAGS) -c $^ -o $@

# Generate two separate images (one for Non-Secure and another for Secure) with
# different linker scripts (as they will have different addresses to locate the code).
# This is to make sure that there is no clash with the symbols.
$(BINARY_S): $(OBJS) $(SRC_ASM) boot.o
	$(CC) $(CFLAGS) $(SECURE_LINKER_ARGS) boot.o $(OBJS) -T $(LINKER_SCRIPT) -o $@
	$(NM) $@ > nm_s.out
	$(OBJ) -D $@ > objdump_s.out

$(BINARY_NS): $(OBJS_NS)
	$(CC) $(BINARY_LIB_S) $^ $(COMMON_CFLAGS) -T $(LINKER_SCRIPT_NS) -o $@
	$(NM) $@ > nm_ns.out
	$(OBJ) -D $@ > objdump_ns.out

# Select the subsystem an505, specify the cortex-m33
# Ctrl-A, then X to quit
run: $(BINARY_S)
	$(QEMU_PATH) \
		-machine $(MACHINE_NAME) \
		-cpu cortex-m33 \
		-m 16M \
		-nographic \
		-semihosting \
		-kernel $(BINARY_S)

gdbserver: $(BINARY)
	$(QEMU_PATH) \
		-machine $(MACHINE_NAME) \
		-cpu cortex-m33 \
		-m 16M \
		-nographic \
		-semihosting \
		-kernel $(BINARY_S) \
		-S -s 

gdb: $(BINARY)
	$(GDB) $(BINARY_S) -ex "target remote:1234"

help:
	$(QEMU_PATH) \
		 --machine help

clean:
	rm -f *.o *.elf *.out $(OBJS)
