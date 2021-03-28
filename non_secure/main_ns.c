#include "logPrint.h"
#include "uart.h"
#include <stddef.h>

extern int sec_sum(void);

/* Non Secure main() */
int main(void) {
    sec_sum();

    while(1);
    return 0;
}
