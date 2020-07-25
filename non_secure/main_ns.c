#include <stddef.h>

extern int sec_sum(int *, size_t);

/* Non Secure main() */
int main(void) {
    int p[256];
    sec_sum(p, 256);

    return 0;
}
