#include "logPrint.h"
#include "ARMCM33_DSP_FP_TZ.h"
#include "partition_ARMCM33.h"

#include <arm_cmse.h>
#include <stdint.h>

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

//void nonsecure_init(void) {
//    /* Set the address of the Vector Table of the Non-Secure */
//    uint32_t *vtor = (uint32_t *) TZ_VTOR_TABLE_ADDR;
//    SCB_NS->VTOR = (uint32_t) vtor;

    /* 
     * Update the Non-Secure Stack Pointer
     * (first element of the Vector Table)
     */
//    __TZ_set_MSP_NS(*vtor);

    /*
     * Jump to the reset handler (Second element of the Vector Table)
     * of the Non-Secure
     */
//    ns_func_void *ns_reset = (ns_func_void*) (*(vtor + 1));
//    ns_reset();
//}

/* Secure main */
int main(void)
{
    logPrint("Start...\n");

    /* Setup the SAU regions */
    TZ_SAU_Setup();

    /* Jump to Non-Secure main address */
    //nonsecure_init();

    while (1) {
        __NOP();
    }
    return -1;
}
