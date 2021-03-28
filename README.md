# cortex-mv8-experiments

Based on
- Arm CoreLink SIE-300 AXI5 System IP for Embedded Technical Reference Manual:
  https://developer.arm.com/documentation/101526/0100/

- AN505 - Example IoT Kit Subsystem Design for a V2M-MPS2+:
  https://developer.arm.com/documentation/dai0505/b/

Clone the project as follow:
```
git clone --recurse-submodules <URL> 
```

## Get QEMU

```
sudo apt-get -y install libglib2.0-dev libpixman-1-dev ninja-build libncurses5

# This is already done through the submodule
##  git clone https://github.com/qemu/qemu.git
cd qemu
mkdir -p build
./configure
make
```

QEMU needs at least the following patches:
http://next.patchew.org/QEMU/20180220180325.29818-1-peter.maydell@linaro.org/20180220180325.29818-2-peter.maydell@linaro.org/

## Get Arm toolchain
 
Get the toolchain from `https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads`.
Tested using `gcc-arm-none-eabi-8-2018-q4-major`.

```
wget "https://developer.arm.com/-/media/Files/downloads/gnu-rm/8-2019q3/RC1.1/gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2?revision=c34d758a-be0c-476e-a2de-af8c6e16a8a2?product=GNU%20Arm%20Embedded%20Toolchain,64-bit,,Linux,8-2019-q3-update" -O gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2

# Extract the archive
tar xvf gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2
```

## Get CMSIS

```
# This is already done through the submodule
##  git clone https://github.com/ARM-software/CMSIS_5.git
```

Tested with commit-id 5865a4a5e511ab094bac46862bd358859ce5ccb4

Linker script is extracted from `$(CMSIS)/Device/ARM/ARMCM33/Source/GCC/gcc_arm.ld`
and adapted for an505. See Chapter "SIE-200 Memory Map Overview" of
Application Note AN505.

## Build and start

- Update path to toolchain, CMSIS and Qemu in the Makefile.
- Compile using `make` and start with `make run`. Note: You might need to adjust
  `CMSIS_PATH`, `QEMU_PATH` and `TOOLCHAIN_PATH` if they are not at the same
  place.
- Debug using `make gdbserver` and (in another terminal) `make gdb`.


# References

- https://arm-software.github.io/CMSIS_5/Zone/html/index.html for MPC configuration
- https://developer.arm.com/documentation/dai0505/latest/ (Example IoT Kit Subsystem Design for a V2M-MPS2+) for the description of the AN505
- https://developer.arm.com/documentation/101104/0200/introduction/about-the-sse-200 Arm CoreLink SSE-200 Subsystem for Embedded Technical Reference Manual
- https://developer.arm.com/documentation/ecm0359818/latest ARMv8-M Security Extensions: Requirements on Development Tools - Engineering Specification
- Arm Cortex-M33 Devices Generic User Guide for Register details


