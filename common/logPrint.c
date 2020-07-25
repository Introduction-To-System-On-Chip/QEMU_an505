#include "logPrint.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

/* Static functions and constant data */
const int SYS_WRITE0 = 0x04;

void semihost(int sys_id, const void *arg)
{
  register int r0 __asm__ ("r0") = sys_id;
  register const void *r1 __asm__ ("r1") = arg;
  __asm__ volatile ("bkpt 0xab");
  (void) r0;
  (void) r1;
}

void logPrint(const char* printString)
{
  semihost(SYS_WRITE0, printString);
}
