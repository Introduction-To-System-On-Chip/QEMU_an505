__STACK_SIZE = 0x00100000;

MEMORY
{
   NS_FLASH    (rx)  : ORIGIN = 0x00000000, LENGTH = 224M
   S_FLASH     (rx)  : ORIGIN = 0x10000000, LENGTH = 2M /* Flash is actually 224M */
   S_FLASH_END (rx)  : ORIGIN = 0x10100000, LENGTH = 1M 
   S_RAM       (rwx) : ORIGIN = 0x20000000, LENGTH = 16M
}

/* Entry Point (reset) */
ENTRY(Reset_Handler)

SECTIONS
{
  .text : 
  {
    /* Make sure the vector is the first point of the project. */
    KEEP(*(.vectors))
    *(.text)
    *(.data)
  	*(.bss) 
  } > S_FLASH

  /*
   * SG veneers:
   * All SG veneers are placed in the special output section .gnu.sgstubs.
   * Its start address must be set, either with the command line option 
   * '--section-start' or in a linker script, to indicate where to place these
   * veneers in memory.
   */
  .gnu.sgstubs :
  {
    _ld_veneer_base = .;
    *(.gnu.sgstubs*)
    . = ALIGN(32);
    _ld_veneer_limit = .;
  } > S_FLASH

  /* Set __StackTop for the vector table first index. */
  __StackTop = ORIGIN(S_FLASH_END) + LENGTH(S_FLASH_END);
  /*
   * Stack is descending, meaning that putting values in the stack will decrease
   * so the limit of the stack is less than the base size address.
   */
  __StackLimit = __StackTop - __STACK_SIZE;
}
