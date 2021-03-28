#include "uart.h"

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

/* Base address of the UART0 See application note AN505 */
#define UART0_BASE_NONSECURE 0x40200000
#define UART0_BASE_SECURE    0x50200000

#ifdef C_SECURE_CODE
#define UART0_BASE UART0_BASE_SECURE
#else
#define UART0_BASE UART0_BASE_NONSECURE
#endif

#define UARTDR(base) (*((volatile uint32_t *)(base + 0x00)))
#define UARTCTRL(base) (*((volatile uint32_t *)(base + 0x08)))

void initUart(void)
{
  UARTCTRL(UART0_BASE) = 1 /* TX_ENABLE	*/;
}

/*
 * This function is a primitive for printf. This is the base function that
 * printf uses.
 */
int __io_putchar(int ch)
{
  UARTDR(UART0_BASE) = ch;
  return ch;
}

void printString(const char* ptr)
{
  while (*ptr != '\0') {
      __io_putchar(*ptr);
      ptr++;
  }
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
  printString(buffer);
}
