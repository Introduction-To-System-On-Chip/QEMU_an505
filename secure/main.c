#include "logPrint.h"
#include "uart.h"
#include "ARMCM33_DSP_FP_TZ.h"
#include "partition_ARMCM33.h"

#include <arm_cmse.h>
#include <stdint.h>

/*------------------- Memory Protection Controller -----------------------------*/
typedef struct /* see "ARM CoreLink SSE-200 Subsystem Technical Reference Manual r1p0" */
{
  __IOM  uint32_t CTRL;                     /* Offset: 0x000 (R/W) Control Register */
         uint32_t RESERVED0[3];
  __IM   uint32_t BLK_MAX;                  /* Offset: 0x010 (R/ ) Block Maximum Register */
  __IM   uint32_t BLK_CFG;                  /* Offset: 0x014 (R/ ) Block Configuration Register */
  __IOM  uint32_t BLK_IDX;                  /* Offset: 0x018 (R/W) Block Index Register */
  __IOM  uint32_t BLK_LUT;                  /* Offset: 0x01C (R/W) Block Lookup Tabe Register */
  __IM   uint32_t INT_STAT;                 /* Offset: 0x020 (R/ ) Interrupt Status Register */
  __OM   uint32_t INT_CLEAR;                /* Offset: 0x024 ( /W) Interrupt Clear Register */
  __IOM  uint32_t INT_EN;                   /* Offset: 0x028 (R/W) Interrupt Enable Register */
  __IM   uint32_t INT_INFO1;                /* Offset: 0x02C (R/ ) Interrupt Info1 Register */
  __IM   uint32_t INT_INFO2;                /* Offset: 0x030 (R/ ) Interrupt Info2 Register */
  __OM   uint32_t INT_SET;                  /* Offset: 0x034 ( /W) Interrupt Set Register */
} MPC_TypeDef;

#define MPC_SRAM0 ((MPC_TypeDef *) 0x58007000) /* Control for 0x[01]0000000 */
//https://developer.arm.com/documentation/101104/0200/programmers-model/base-element/secure-privilege-control-block
#define SPCBlock_Base ((uint32_t*)0x58008000)

void printCmseAddressInfo(uint32_t addr)
{
    cmse_address_info_t c = cmse_TTA(addr);
    logPrint("Address: 0x%x\n", addr);
    logPrint(" |mpu_region: %x\n", c.flags.mpu_region);
    logPrint(" |sau_region: %x\n", c.flags.sau_region);
    logPrint(" |mpu_region_valid: %x\n", c.flags.mpu_region_valid);
    logPrint(" |sau_region_valid: %x\n", c.flags.sau_region_valid);
    logPrint(" |read_ok: %x\n", c.flags.read_ok);
    logPrint(" |readwrite_ok: %x\n", c.flags.readwrite_ok);
    logPrint(" |nonsecure_read_ok: %x\n", c.flags.nonsecure_read_ok);
    logPrint(" |nonsecure_readwrite_ok: %x\n", c.flags.nonsecure_readwrite_ok);
    logPrint(" |secure: %x\n", c.flags.secure);
    logPrint(" |idau_region_valid: %x\n", c.flags.idau_region_valid);
    logPrint(" |idau_region: %x\n", c.flags.idau_region);
}

void __aeabi_unwind_cpp_pr0()
{
}

/* Non-Secure Callable functions */
typedef int __attribute__((cmse_nonsecure_call)) ns_func_void(void);

void __attribute__((cmse_nonsecure_entry)) sec_sum(void)
{
  logPrint("Called secure function!!\n");
}

void nonsecure_init(void) {
  /* Set the address of the Vector Table of the Non-Secure */
  uint32_t *vtor = (uint32_t *) TZ_VTOR_TABLE_ADDR;
  SCB_NS->VTOR = (uint32_t) 0x10200000;

  /* 
   * Update the Non-Secure Stack Pointer
   * (first element of the Vector Table)
   */
  __TZ_set_MSP_NS(*vtor);
  __TZ_set_PRIMASK_NS(0x0);

  /*
   * Jump to the reset handler (Second element of the Vector Table)
   * of the Non-Secure
   */
    ns_func_void *ns_reset = (ns_func_void*) (*(vtor + 1));
    printCmseAddressInfo(ns_reset);

    ns_reset();
}

void initMpc(void)
{
  uint32_t blockCtlr = MPC_SRAM0->CTRL;
  uint32_t blockMax = MPC_SRAM0->BLK_MAX;
  uint32_t blockSize = MPC_SRAM0->BLK_CFG & (0b1111); 
  uint32_t blockSizeByte = 1 << (blockSize + 5);

  logPrint("Config: %x\n", blockCtlr);
  logPrint(" | block max: %x\n", blockMax);
  logPrint(" | block size: 0x%x (0x%x bytes)\n", blockSize, blockSizeByte);

  uint32_t addrStartRam = 0x00200000;
  uint32_t addrStopRam  = 0x003fffff;
  // Find the corresponding group id.
  // BLK_IDX is the index of the group. If N, it selects Blocks
  //  [32 * N .. (32 * N+1) - 1] where 32 is the number of bits of the register.
  // Each block is M bytes depending on the block size where
  //  Block size = 1 << (BLK_CFG+5)
  // Which means that if we have address X, finding the corresponding
  // block is done by:
  //  Y = (X % 32) will be the bit index in the 32bit register
  //  Z = (Y % blockSize) will 
  uint32_t startGroup = (addrStartRam >> (blockSize + 5)) / 32;
  uint32_t endGroup = (addrStopRam >> (blockSize + 5)) / 32;

  if (endGroup > blockMax)
  {
    endGroup = blockMax;
  }
  logPrint("startGroup: 0x%x\n", startGroup);
  logPrint("endGroup: 0x%x\n", endGroup);

  MPC_SRAM0->CTRL |= 1 << 4;
  MPC_SRAM0->CTRL &= ~(1 << 8); // No autoincrement
  for (uint32_t i = startGroup; i <= endGroup; i++)
  {
    MPC_SRAM0->BLK_IDX = i;
    MPC_SRAM0->BLK_LUT = 0xffffffff;
  }

  MPC_SRAM0->CTRL |= (1 << 8); // autoincrement
  MPC_SRAM0->BLK_IDX = 0;
  for (uint32_t i = 0; i <= blockMax; i++)
  {
    logPrint("Value[0x%x]>", i);
    logPrint(" 0x%x-0x%x:\t\t",
             i * 32 * blockSizeByte,
             ((i+1) * (32*blockSizeByte)) - 1);
    logPrint("0x%x\n", MPC_SRAM0->BLK_LUT);
  }

  MPC_SRAM0->CTRL &= ~(1 << 8); // No autoincrement
}

/* Secure main */
int main(void)
{
    SystemInit();
    initUart();
    printString("Start\n");
 
    volatile uint32_t* spcb = 0x50080000 + 0x14;
    *spcb |= 1 | 2;
    logPrint("SPCB content 0x%x\n", *spcb);
   
    initMpc();

    /* 
    
    The Non-secure Callable Configuration register allows software to define
     callable regions of memory. The register can do this if the Secure Code
     region is 0x1000_0000 to 0x1FFF_FFFF and the Secure RAM region is
      0x3000_0000 to 0x3FFF_FFFF.
    */
    *spcb |= 1 | 2;
    logPrint("SPCB content 0x%x\n", *spcb);

    printCmseAddressInfo(0x00000000);
    printCmseAddressInfo(0x00200000);
    printCmseAddressInfo(0x10000000);
    printCmseAddressInfo(0x20000000);
    printCmseAddressInfo(SAU_INIT_START2);
    printCmseAddressInfo(SAU_INIT_END2);
    /* Jump to Non-Secure main address */
    nonsecure_init();

    while (1)
    {
      __NOP();
      logPrint("Should not happen..\n");
    }
    return -1;
}
