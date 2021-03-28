#include "logPrint.h"
#include "uart.h"
#include <stddef.h>

extern int sec_sum(void);

/* Non Secure main() */
int main(void) {
    sec_sum();

    return 0;
}
