#include <ARMCM33_DSP_FP_TZ.h>
#include <arm_cmse.h>

/* Static functions and constant data */
const int SYS_WRITE0 = 0x04;
static void semihost(int sys_id, const void *arg)
{
    register int r0 __asm__ ("r0") = sys_id;
    register const void *r1 __asm__ ("r1") = arg;
    __asm__ volatile ("bkpt 0xab");
    (void) r0;
    (void) r1;
}

/* Non-Secure Callable functions */
typedef int __attribute__((cmse_nonsecure_call)) ns_func_void(void);

void __attribute__((cmse_nonsecure_entry)) sec_sum(int *p, size_t s)
{
    semihost(SYS_WRITE0, "In security function.\n");
    for (size_t i = 0; i < s; i++) {
        p[i] = i;
    }
}

void nonsecure_init(void) {
    /* Set the address of the Vector Table of the Non-Secure */
    uint32_t *vtor = (uint32_t *) TZ_VTOR_TABLE_ADDR;
    SCB_NS->VTOR = (uint32_t) vtor;

    /* 
     * Update the Non-Secure Stack Pointer
     * (first element of the Vector Table)
     */
    __TZ_set_MSP_NS(*vtor);

    /*
     * Jump to the reset handler (Second element of the Vector Table)
     * of the Non-Secure
     */
    ns_func_void *ns_reset = (ns_func_void*) (*(vtor + 1));
    ns_reset();
}

/* Secure main */
int main(void)
{
    semihost(SYS_WRITE0, "Start\n");
    /* Jump to Non-Secure main address */
    nonsecure_init();
    while (1) {
        __NOP();
    }
    return -1;
}
