#include "logPrint.h"
#include "uart.h"
#include "ARMCM33_DSP_FP_TZ.h"
#include "partition_ARMCM33.h"

#include <arm_cmse.h>
#include <stdint.h>

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

void __attribute__((cmse_nonsecure_entry)) sec_sum(int *p, size_t s)
{
  for (size_t i = 0; i < s; i++)
  {
    p[i] = i;
  }
}

void nonsecure_init(void) {
  /* Set the address of the Vector Table of the Non-Secure */
  uint32_t *vtor = (uint32_t *) TZ_VTOR_TABLE_ADDR;
  SCB_NS->VTOR = (uint32_t) 0x00200000;

  /* 
   * Update the Non-Secure Stack Pointer
   * (first element of the Vector Table)
   */
  //__TZ_set_MSP_NS(*vtor);
  __TZ_set_MSP_NS(0x00800000);
  __TZ_set_PRIMASK_NS(0x0);

  /*
   * Jump to the reset handler (Second element of the Vector Table)
   * of the Non-Secure
   */
    //ns_func_void *ns_reset = (ns_func_void*) (*(vtor + 1));
    ns_func_void *ns_reset = (ns_func_void*) (0x00200825);
    cmse_address_info_t c = cmse_TTA(ns_reset);
    ns_reset();
}

void initMpc(void)
{
  /* See corelink_sie300_axi5_system_ip_for_embedded_technical_reference_manual */
  const uint8_t* baseSram1_MPC = 0x58007000; /* Control for 0x[01]0000000 */
  const uint8_t* baseSram2_MPC = 0x58008000; /* Control for 0x[12]0000000 */
  const uint8_t* baseSram3_MPC = 0x58009000; /* Control for 0x[23]0000000 */
  
  uint32_t offset_CTRL    = 0x00;
  uint32_t offset_BLK_MAX = 0x10;
  uint32_t offset_CFG     = 0x14;
  uint32_t offset_IDX     = 0x18;
  uint32_t offset_LUT     = 0x1C;

  uint32_t* mpc1_Ctrl = baseSram1_MPC + offset_CTRL;
  uint32_t* mpc1_Lut = baseSram1_MPC + offset_LUT;
  uint32_t* mpc1_Idx = baseSram1_MPC + offset_IDX;

  uint32_t* mpc3_Ctrl = baseSram3_MPC + offset_CTRL;
  uint32_t* mpc3_Lut = baseSram3_MPC + offset_LUT;
  uint32_t* mpc3_Idx = baseSram3_MPC + offset_IDX;



  uint32_t blockCtlr = *(baseSram1_MPC + offset_CTRL);
  uint32_t blockMax = *(baseSram1_MPC + offset_BLK_MAX);
  uint32_t blockSize = *(baseSram1_MPC + offset_CFG);
  uint32_t blockIdx = *(baseSram1_MPC + offset_IDX);
  uint32_t blockLut = *(baseSram1_MPC + offset_LUT);


  /* for mpc1: from (0x00000000 + 0x200000 =) 0x00200000 to 0x00204000 */
  //uint32_t addrStartRam = 0x200000U;
  //uint32_t addrStopRam  = 0x204000U;
  
  
  uint32_t addrStartRam = 0x00200000;
  uint32_t addrStopRam  = 0x00201000;
  uint32_t startGroup = (addrStartRam >> blockSize) >> 0x5;
  uint32_t endGroup = (addrStopRam >> blockSize) >> 0x5;

  logPrint("startGroup: %x\n", startGroup);
  logPrint("endGroup: %x\n", endGroup);

  *mpc1_Ctrl |= 1 << 4;

  printCmseAddressInfo(mpc1_Lut);

  for (uint32_t i = startGroup; i < endGroup; i++)
  {
    *mpc1_Idx = i;
    uint32_t valueBefore = *mpc1_Lut;
    *mpc1_Lut = 0xffffffff;
    logPrint("mpc1_Lut: %x %x\n", *mpc1_Lut, valueBefore);
  }

  *mpc3_Ctrl |= 1 << 4;
  for (uint32_t i = startGroup; i < endGroup; i++)
  {
    *mpc3_Idx = i;
    uint32_t valueBefore = *mpc3_Lut;
    *mpc3_Lut = 0xffffffff;
    logPrint("mpc3_Lut: %x %x\n", *mpc3_Lut, valueBefore);
  }

}

/* Secure main */
int main(void)
{
    SystemInit();
    initUart();
    printString("Start\n");

    printCmseAddressInfo(0x00000000);
    printCmseAddressInfo(0x00200000);
    printCmseAddressInfo(0x10000000);
    printCmseAddressInfo(0x20000000);
    printCmseAddressInfo(0x30000000);

    uint32_t* addrCPUID = 0xE002ED00;
    uint32_t valueCPUID = *addrCPUID;
    
    initMpc();

    /* Jump to Non-Secure main address */
    nonsecure_init();

    while (1)
    {
      __NOP();
    }
    return -1;
}
