# cortex-mv8-experiments

`sudo apt-get install libncurses5`

## Get QEMU

```
sudo apt-get -y install libglib2.0-dev
sudo apt-get install libpixman-1-dev

git clone https://github.com/qemu/qemu.git
cd qemu
mkdir -p build
./configure
make
```

## Get Arm toolchain
 
Get the toolchain from `https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads`. Tested using gcc-arm-none-eabi-8-2018-q4-major.

```
wget "https://developer.arm.com/-/media/Files/downloads/gnu-rm/8-2019q3/RC1.1/gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2?revision=c34d758a-be0c-476e-a2de-af8c6e16a8a2?product=GNU%20Arm%20Embedded%20Toolchain,64-bit,,Linux,8-2019-q3-update" -O gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2

tar xvf gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2
```

## Get CMSIS

```
git clone https://github.com/ARM-software/CMSIS_5.git
```

Tested with commit-id 5865a4a5e511ab094bac46862bd358859ce5ccb4

Linker script is extracted from `$(CMSIS)/Device/ARM/ARMCM33/Source/GCC/gcc_arm.ld`
and adapted for an505. See Chapter "SIE-200 Memory Map Overview" of
Application Note AN505.

## Build and start

- Update path to toolchain, CMSIS and Qemu in the Makefile.
- Compile using `make` and start with `make run`.
- Debug using `make gdbserver` and (in another terminal) `make gdb`.
