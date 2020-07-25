BINARY_S = secure.elf
BINARY_LIB_S = cmse_import.o
BINARY_NS = nonsecure.elf
BINARY_ALL = image_s_ns.elf

MACHINE_NAME := mps2-an505

CMSIS_PATH ?= ../CMSIS_5
QEMU_PATH ?= ../qemu/arm-softmmu/qemu-system-arm
TOOLCHAIN_PATH ?= ../gcc-arm-none-eabi-8-2019-q3-update/bin

CROSS_COMPILE = $(TOOLCHAIN_PATH)/arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
GDB = $(CROSS_COMPILE)gdb
OBJ = $(CROSS_COMPILE)objdump
NM = $(CROSS_COMPILE)nm
READELF = $(CROSS_COMPILE)readelf

LINKER_SCRIPT_NS = non_secure/gcc_arm_ns.ld
# Must match the same value in the NS linker script where the vector table is.
TZ_VTOR_TABLE_ADDR = 0x20000000
LINKER_SCRIPT = secure/gcc_arm.ld

# From $(CMSIS_PATH)/Device/ARM/ARMCM33/Source/GCC/startup_ARMCM33.S
SRC_ASM = secure/startup_ARMCM33.S
# From $(CMSIS_PATH)/Device/ARM/ARMCM33/Source/system_ARMCM33.c
SRC_C = \
    secure/system_ARMCM33.c \
	  secure/main.c \
	  common/logPrint.c

INCLUDE_FLAGS = \
    -I$(CMSIS_PATH)/Device/ARM/ARMCM33/Include \
  	-I$(CMSIS_PATH)/CMSIS/Core/Include \
  	-I./secure \
  	-I./non_secure \
  	-I./common

COMMON_CFLAGS = \
    -mcpu=cortex-m33 \
    -g \
    $(INCLUDE_FLAGS) \
    -nostartfiles -ffreestanding \
    -mthumb

CFLAGS = \
    $(COMMON_CFLAGS) \
    -DARMCM33_DSP_FP_TZ \
    -mcmse \
    -DTZ_VTOR_TABLE_ADDR=$(TZ_VTOR_TABLE_ADDR)

CFLAGS_NS = \
    $(COMMON_CFLAGS) \
    --specs=nosys.specs -DARMCM33

SECURE_LINKER_ARGS = \
    -Xlinker --sort-section=alignment \
    -Xlinker --cmse-implib \
    -Xlinker --out-implib=$(BINARY_LIB_S)

OBJS = \
    secure/main.o \
    common/logPrint.o \
    secure/system_ARMCM33.o \
    secure/boot.o

OBJS_NS = \
    non_secure/main_ns.o \
    non_secure/system_ARMCM33_ns.o \
    non_secure/boot_ns.o \
    $(BINARY_LIB_S)

all: $(BINARY_S) $(BINARY_NS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

secure/boot.o: $(SRC_ASM)
	$(CC) $(CFLAGS) -c $^ -o $@

non_secure/boot_ns.o: non_secure/boot_ns.S
	$(CC) $(CFLAGS_NS) -c $^ -o $@

# Generate two separate images (one for Non-Secure and another for Secure) with
# different linker scripts (as they will have different addresses to locate the code).
# This is to make sure that there is no clash with the symbols.
$(BINARY_S): $(OBJS)
	$(CC) $(CFLAGS) $(SECURE_LINKER_ARGS) $^ -T $(LINKER_SCRIPT) -o $@
	$(NM) $@ > secure/nm_s.out
	$(OBJ) -D $@ > secure/objdump_s.out
	$(READELF) -a $@ > secure/readelf_s.out

$(BINARY_NS): $(OBJS_NS)
	$(CC) $^ $(CFLAGS_NS) -T $(LINKER_SCRIPT_NS) -o $@
	$(NM) $@ > non_secure/nm_ns.out
	$(OBJ) -D $@ > non_secure/objdump_ns.out
	$(READELF) -a $@ > non_secure/readelf_ns.out

# Select the subsystem an505, specify the cortex-m33
# Ctrl-A, then X to quit
run: $(BINARY_S)
	$(QEMU_PATH) \
		-machine $(MACHINE_NAME) \
		-cpu cortex-m33 \
		-m 16M \
		-nographic \
		-semihosting \
		-device loader,file=$(BINARY_NS) \
		-device loader,file=$(BINARY_S)

gdbserver: $(BINARY)
	$(QEMU_PATH) \
		-machine $(MACHINE_NAME) \
		-cpu cortex-m33 \
		-m 16M \
		-nographic \
		-semihosting \
		-device loader,file=$(BINARY_NS) \
		-device loader,file=$(BINARY_S) \
		-S -s 

gdb: $(BINARY)
	$(GDB) $(BINARY_S) -ex "target remote:1234"

help:
	$(QEMU_PATH) --machine help

clean:
	rm -f *.o *.elf *.out secure/*.out non_secure/*.out $(OBJS)
