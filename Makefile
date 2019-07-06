BINARY := image.elf

TOOLCHAIN = ../gcc-arm-none-eabi-8-2018-q4-major-linux/gcc-arm-none-eabi-8-2018-q4-major/bin
CROSS_COMPILE = $(TOOLCHAIN)/arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
GDB = $(CROSS_COMPILE)gdb
OBJ = $(CROSS_COMPILE)objdump

CMSIS = ../CMSIS_5
QEMU_PATH = /usr/local/bin/qemu-system-arm

LINKER_SCRIPT = gcc_arm.ld

SRC_ASM = $(CMSIS)/Device/ARM/ARMCM33/Source/GCC/startup_ARMCM33.S
SRC_C = $(CMSIS)/Device/ARM/ARMCM33/Source/system_ARMCM33.c \
	main.c

INCLUDE_FLAGS = -I$(CMSIS)/Device/ARM/ARMCM33/Include \
	-I$(CMSIS)/CMSIS/Core/Include \
	-I. \

CFLAGS = -mcpu=cortex-m33 \
	-g3 \
	--specs=nosys.specs \
	-Wall \
	$(INCLUDE_FLAGS) \
	-DARMCM33_DSP_FP_TZ \
	-mcmse

OBJS = main.o boot.o startup_ARMCM33.o system_ARMCM33.o

all: $(BINARY)

boot.o: $(SRC_ASM)
	$(CC) $(CFLAGS) -c $^ -o $@

$(BINARY): $(SRC_C) $(SRC_ASM)
	$(CC) $^ $(CFLAGS) -T $(LINKER_SCRIPT) -o $@
	$(OBJ) -D $@ > objdump

# Select the subsystem an521, specify the cortex-m33
# Ctrl-A, then X to quit
run: $(BINARY)
	$(QEMU_PATH) \
		-machine mps2-an521 \
		-cpu cortex-m33 \
		-m 4096 \
		-nographic \
		-semihosting \
		-kernel $(BINARY)

gdbserver: $(BINARY)
	$(QEMU_PATH) \
		-machine mps2-an521 \
		-cpu cortex-m33 \
		-m 4096 \
		-nographic -serial mon:stdio \
		-kernel $(BINARY) \
		-S -s 

help:
	$(QEMU_PATH) \
		 --machine help

gdb: $(BINARY)
	$(GDB) $(BINARY) -ex "target remote:1234"

clean:
	rm -f *.o *.elf
