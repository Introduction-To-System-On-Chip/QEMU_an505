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

void logPrint(const char* format, ...)
{
  char buffer[256] = {'\0'};
  int bufferPos = 0;

  unsigned int hex;

  va_list arg;
  va_start(arg, format);

  for(const char* nextChar = format; *nextChar != '\0'; nextChar++)
  {
    while ((*nextChar != '%') && (*nextChar != '\0'))
    {
      sprintf(buffer + bufferPos, "%c", *nextChar);
      bufferPos++;
      nextChar++;
    }

    if (*nextChar == '\0')
    {
      break;
    }

    nextChar++;

    switch (*nextChar)
    {
      case 'x':
        hex = va_arg(arg, unsigned int);
        int written = 0;
        written = sprintf(buffer + bufferPos, "%x", hex);
        bufferPos = bufferPos + written;
        break;
    }
  }

  buffer[bufferPos] = '\0';
  va_end(arg);
  semihost(SYS_WRITE0, buffer);
}

