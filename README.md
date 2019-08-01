# cortex-mv8-experiments

## Get QEMU

```
git clone https://github.com/qemu/qemu.git
cd qemu
mkdir -p build
./configure
make
```

## Get Arm toolchain
 
Get the toolchain from `https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads`. Tested using gcc-arm-none-eabi-8-2018-q4-major.

## Get CMSIS

```
git clone https://github.com/ARM-software/CMSIS_5.git
```

Linker script is extracted from `$(CMSIS)/Device/ARM/ARMCM33/Source/GCC/gcc_arm.ld`
and adapted for an521. See Chapter "SIE-200 Memory Map Overview" of
Application Note AN521.

## Build and start

- Update path to toolchain, CMSIS and Qemu in the Makefile.
- Compile using `make` and start with `make run`.
- Debug using `make gdbserver` and (in another terminal) `make gdb`.
