#include "hw1.h"
#include "debug.h"
#include "const.h"
#include "helpers.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the program
 * and will return a unsigned short (2 bytes) that will contain the
 * information necessary for the proper execution of the program.
 *
 * IF -p is given but no (-r) ROWS or (-c) COLUMNS are specified this function
 * MUST set the lower bits to the default value of 10. If one or the other
 * (rows/columns) is specified then you MUST keep that value rather than assigning the default.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return Refer to homework document for the return value of this function.
 */
unsigned short validargs(int argc, char **argv) {
    char *p1, *p2, next  = 0, polybius = 0;
    unsigned short mode  = 0;
    unsigned short psize = len(polybius_alphabet);
    // unsigned short fsize = len(fm_alphabet);

    // no flags provided
    if (argc == 1) return 0;

    // get first positional arg
    p1 = *(argv + ++next);

    // -h flag?
    if (scomp(p1, _H)) return 0x8000;  // set bit 15

    // must be -f|-p followed by -e|-d
    if (argc < MIN_AS) return 0;   // too few args to do anything

    p2 = *(argv + ++next);
    // debug("arg [%d]:%s", next, p2);

    // identify cipher type
    if (scomp(p1, _P)) {
        // clear bit 14
        setbit(&mode, 14, 0);   // 0x0000
        // set -r and -c to default values
        mode |= 0x00AA;
        polybius = 1;
        if (100 < psize) {
            debug("default row*column size: %d size too small for alphabet size %d", 100, psize);
            return 0;
        }
    }
    else if (scomp(p1, _F)) setbit(&mode, 14, 1);   // 0x4000
    else return 0;   // not -p|-f error

    if (scomp(p2, _E)) setbit(&mode, 13, 0);        // 0x0000 || 0x2000
    else if (scomp(p2, _D)) setbit(&mode, 13, 1);   // 0x4000 || 0x6000
    else return 0;   // not -e|-d error

    // read opt args
    if (argc > MIN_AS) {
        // read in pairs
        char *opt, oarg;
        char r = 1, c = 1, k = 1;       // switch to 0 when arg is read.
        while (next < argc - 1) {
            opt = *(argv + ++next);
            ++next;
            if (scomp(opt, _K) && k) {
                // check key
                opt = *(argv + next);
                if (polybius && valkey(opt, polybius_alphabet)) {
                    key = opt; // set global polybius key
                }
                else if (!polybius && valkey(opt, fm_alphabet)) {
                    key = opt; // set global fm key
                }
                else {
                    debug("invalid key: %s", opt);
                    return 0;
                }
                k = 0;          // k is read
            }
            else if (polybius && scomp(opt, _R) && r) {
                // check row count if -p
                oarg = strtonum(*(argv + next));
                if (oarg < 9 || oarg > 16) {
                    debug("invalid number of rows: %d", oarg);
                    return 0;
                }
                mode ^= 0x00A0;     // undo def value
                mode |= (oarg << 4);
                r = 0;
            }
            else if (polybius && scomp(opt, _C) && c) {
                // check column count if -p
                oarg = strtonum(*(argv + next));
                if (oarg < 9 || oarg > 16) {
                    debug("invalid number of cols: %d", oarg);
                    return 0;
                }
                mode ^= 0x000A;
                mode |= oarg;
                c = 0;
            }
            else return 0;   // unrecognized flag
        }
        short rc = ((mode & 0x00f0) >> 4) * (mode & 0x000f);
        // test if polybius row * col < alphabet size
        if (polybius && rc < psize) {
            debug("parsed row*column size: %d size too small for alphabet size %d", rc, psize);
            return 0;
        }
    }
    return mode;
}

char valkey(const char *keyV, const char *alphabet) {
    // check if key has dupes
    if (hasdupes(keyV)) return 0;
    char found = 0;
    const char *p = alphabet;
    while (*keyV) {
        found = 0;
        while (*alphabet) {
            // debug("key char %c comparing with %c", *keyV, *alphabet);
            if (*keyV == *alphabet) found = 1;
            alphabet++;
        }
        if (!found) return 0;
        alphabet = p;
        keyV++;
    }
    return 1;
}
