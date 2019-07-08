BINARY_S := secure.elf
BINARY_LIB_S := importlib_v1.o
BINARY_NS := nonsecure.elf
BINARY_ALL := image_s_ns.elf

TOOLCHAIN = ../gcc-arm-none-eabi-8-2018-q4-major-linux/gcc-arm-none-eabi-8-2018-q4-major/bin
CROSS_COMPILE = $(TOOLCHAIN)/arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
GDB = $(CROSS_COMPILE)gdb
OBJ = $(CROSS_COMPILE)objdump
NM = $(CROSS_COMPILE)nm

CMSIS = ../CMSIS_5
QEMU_PATH = /usr/local/bin/qemu-system-arm

TZ_VTOR_TABLE_ADDR = 0x00000000
LINKER_SCRIPT = gcc_arm.ld
LINKER_ARGS_S = \
	-Xlinker --defsym=__ROM_BASE=0x10000000 \
	-Xlinker --defsym=__ROM_SIZE=0x00040000 \
	-Xlinker --defsym=__RAM_BASE=0x38000000 \
	-Xlinker --defsym=__RAM_SIZE=0x00020000 \
	-Xlinker --cmse-implib \
	-Xlinker --sort-section=alignment \
	-Xlinker --out-implib=$(BINARY_LIB_S)

# For simplicity, re-use the same linker script but update the base addresses
LINKER_SCRIPT_NS = gcc_arm_ns.ld
LINKER_ARGS_NS = \
	-Xlinker --defsym=__ROM_BASE=$(TZ_VTOR_TABLE_ADDR) \
	-Xlinker --defsym=__ROM_SIZE=0x00040000 \
	-Xlinker --defsym=__RAM_BASE=0x28000000 \
	-Xlinker --defsym=__RAM_SIZE=0x00020000

SRC_ASM = $(CMSIS)/Device/ARM/ARMCM33/Source/GCC/startup_ARMCM33.S
SRC_C = $(CMSIS)/Device/ARM/ARMCM33/Source/system_ARMCM33.c \
	main.c

SRC_C_NS = $(CMSIS)/Device/ARM/ARMCM33/Source/system_ARMCM33.c \
	main_ns.c

INCLUDE_FLAGS = -I$(CMSIS)/Device/ARM/ARMCM33/Include \
	-I$(CMSIS)/CMSIS/Core/Include \
	-I.

CFLAGS = -mcpu=cortex-m33 \
	-g3 \
	--specs=nosys.specs \
	-Wall \
	$(INCLUDE_FLAGS) \
	-DARMCM33_DSP_FP_TZ \
	-DTZ_VTOR_TABLE_ADDR=$(TZ_VTOR_TABLE_ADDR) \
	-mcmse

CFLAGS_NS = -mcpu=cortex-m33 \
	-g3 \
	--specs=nosys.specs \
	-Wall \
	$(INCLUDE_FLAGS) \
	-DARMCM33_DSP_FP_TZ

all: $(BINARY_NS)

boot.o: $(SRC_ASM)
	$(CC) $(CFLAGS) -c $^ -o $@

# Generate two separate images (one for Non-Secure and another for Secure) with
# different linker scripts (as they will have different addresses to locate the code).
# This is to make sure that there is no clash with the symbols.
$(BINARY_S): $(SRC_C) $(SRC_ASM)
	$(CC) $^ $(CFLAGS) -T $(LINKER_SCRIPT) $(LINKER_ARGS_S) -o $@
	$(NM) $@ > nm_s
	$(OBJ) -D $@ > objdump_s

$(BINARY_NS): $(SRC_C_NS) $(BINARY_S) $(SRC_ASM)
	$(CC) $(SRC_C_NS) $(SRC_ASM) $(BINARY_LIB_S) $(CFLAGS_NS) \
		-T $(LINKER_SCRIPT_NS) $(LINKER_ARGS_NS) -o $@
	$(OBJ) -D $@ > objdump_ns

$(BINARY_ALL): $(BINARY_NS) $(BINARY_S)
	srec_cat \
		$(BINARY_NS) -Binary -offset 0x00000000 \
		$(BINARY_S)  -Binary -offset 0x10000000 \
		-o $@ -Intel


# Select the subsystem an521, specify the cortex-m33
# Ctrl-A, then X to quit
run: $(BINARY_S)
	$(QEMU_PATH) \
		-machine mps2-an521 \
		-cpu cortex-m33 \
		-m 4096 \
		-nographic \
		-semihosting \
		-kernel $(BINARY_S) \
		-device loader,file=$(BINARY_NS),addr=0x10000000

gdbserver: $(BINARY)
	$(QEMU_PATH) \
		-machine mps2-an521 \
		-cpu cortex-m33 \
		-m 4096 \
		-nographic \
		-semihosting \
		-kernel a.out \
		-S -s 

help:
	$(QEMU_PATH) \
		 --machine help

gdb: $(BINARY)
	$(GDB) $(BINARY) -ex "target remote:1234"

clean:
	rm -f *.o *.elf
