#include <ARMCM33_DSP_FP_TZ.h>
#include <arm_cmse.h>

const int SYS_WRITE0 = 0x04;
static void semihost(int sys_id, const void *arg)
{
    register int r0 __asm__ ("r0") = sys_id;
    register const void *r1 __asm__ ("r1") = arg;
    __asm__ volatile ("bkpt 0xab");
    (void) r0;
    (void) r1;
}

int main(void)
{
    semihost(SYS_WRITE0, "Start\n");
    while (1) {
        __NOP();
    }

    return 0;
}
