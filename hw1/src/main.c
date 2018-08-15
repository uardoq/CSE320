#include <stdlib.h>

#include "hw1.h"
#include "debug.h"
#include "const.h"
#include "polybius.h"
#include "morse.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    unsigned short mode;
    mode = validargs(argc, argv);

    debug("Mode: 0x%X", mode);

    // help menu
    if (mode & 0x8000) {
        USAGE(*argv, EXIT_SUCCESS);
    }
    // error
    else if (!mode) {
        USAGE(*argv, EXIT_FAILURE);
    }
    // morse
    else if (mode & 0x4000) {
        create_morse_key();
        if (mode & 0x2000) {
            debug("%s", "morse decryption");
            return morse_decrypt();
        }
        else {
            debug("%s", "morse encryption");
            return morse_encrypt();
        }
    }
    // polybius
    else {
        unsigned char rows = (mode & 0x00F0) >> 4;
        unsigned char cols = mode & 0x000F;
        create_polybius_table(rows, cols);
        if (mode & 0x2000) {
            debug("%s", "polybius decryption");
            return polybius_decrypt(rows, cols);
        }
        else {
            debug("%s", "polybius encryption");
            return polybius_encrypt(rows, cols);
        }

    }
    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */