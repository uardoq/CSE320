#include <stdio.h>
#include "sfmm.h"

#include "helpers.h"

int main(int argc, char const *argv[]) {
    // simple use example

    // init
    sf_mem_init();

    // request 23 bytes...
    // but becomes a 48 byte block due to padding and overhead of header/footer (16 bytes total)
    // so after page request... free block = 4096 - 48 = 4048
    double *ptr0 = (double*)sf_malloc(23);

    printf("%s\n", "print the block we just requested...");
    sf_varprint(ptr0);

    printf("%s\n", ">> before freeing...");
    // shows (0xfd0 or 4048) free in free list 4 (because the range of list 4 is 2049 and larger)
    sf_snapshot();

    // free up the 48 bytes
    sf_free(ptr0);

    printf("%s\n", ">> after freeing...");
    // show last state of allocator
    sf_snapshot();

    // clean up
    sf_mem_fini();

    return EXIT_SUCCESS;
}
