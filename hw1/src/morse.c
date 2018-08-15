#include <stdlib.h>
#include <stdio.h>
#include "morse.h"
#include "const.h"
#include "helpers.h"

const char* translate_to_morse_code(char c) {
    // debug("translating char %c to morse code\n", c);
    unsigned char ascii_index = c - START_ASCII;
    const char *morse_code    = *(morse_table + ascii_index);
    return morse_code;
}

void create_morse_key() {
    unsigned char e          = 0;
    const char *alphabet_ptr = fm_alphabet;
    const char *key_ptr      = key;
    char *fm_key_ptr         = fm_key;
    // key specified
    if (key) {
        debug("creating morse key/alphabet with input key, %s\n", key);
        // put key at begining of fm_key
        while (*key_ptr) {
            *fm_key_ptr = *key_ptr;
            key_ptr++;
            fm_key_ptr++;
            e++;
        }
        // put the rest of the alphabet in fm_key
        key_ptr = key;
        unsigned char current_char = 0, next_char = 0;
        while (*alphabet_ptr) {
            next_char = get_next_char(key, current_char);
            if (*alphabet_ptr == next_char) {
                current_char = next_char;
                key_ptr++;
            }
            else {
                *fm_key_ptr = *alphabet_ptr;
                fm_key_ptr++;
                e++;
            }
            alphabet_ptr++;
        }

    }
    // key not specified
    else {
        debug("%s", "creating morse key/alphabet without input key");
        while (*alphabet_ptr) {
            *fm_key_ptr = *alphabet_ptr;
            fm_key_ptr++;
            alphabet_ptr++;
        }
    }
    debug("fm_key: %s", fm_key);

}

char cipher(char *buffer) {
    debug("morse code buffer: %s", buffer);

    long _buffer = 0;
    char *_buffer_ptr = (char*)&_buffer;
    char *_buffer_ptr_base = _buffer_ptr;
    char i = 0;
    char c = 0;
    char *p = buffer;
    char *buffer_ptr_base = p;

    const char **fractionated_table_ptr = fractionated_table;

    // copy first 3 chars of buffer to a temp buffer
    while (i < 3) {
        *_buffer_ptr = *p;
        _buffer_ptr++;
        p++;
        i++;
    }
    // *(++_buffer_ptr) = 0;
    debug("first 3 chars of buffer: %s", _buffer_ptr_base);
    i = 0;
    while (*fractionated_table_ptr) {
        // debug("ftable: %s == buffer_ptr_base: %s", *fractionated_table_ptr, _buffer_ptr_base);
        if (scomp(_buffer_ptr_base, *fractionated_table_ptr)) {
            c = *(fm_key + i);
            debug("cipher char: %c", c);
            // debug("before buffer: %s", buffer_ptr_base);
            shift_chars(buffer_ptr_base, 3);
            // debug("after buffer: %s", buffer_ptr_base);
            break;
        }
        fractionated_table_ptr++;
        i++;
    }
    // debug("cipher output %c", c);
    return c;
}

char morse_encrypt() {
    char c = 0, prev_c = 10;
    char *morse_code;
    char *polybius_table_ptr = polybius_table;

    while (c != -1) {
        c = getchar();
        if (c == 10) {
            // if no input, dont exit, instead keep pulling input
            if (prev_c != 32 && prev_c != 9 && prev_c != 10) {
                strappend(polybius_table_ptr, "x");
                while (len(polybius_table) >= 3) {
                    // debug("len %d", len(buffer_ptr_base));
                    putchar(cipher(polybius_table_ptr));
                }
                // debug("buffer_ptr: %s", buffer_ptr);
                prev_c = c;
            }
            // clear buffer
            int i = 0;
            while (*(polybius_table + i)) {
                *(polybius_table + i) = 0;
            }
            putchar('\n');
        }
        else if (c == 32) {
            if (prev_c != 32 && prev_c != 9) {
                // space
                strappend(polybius_table_ptr, "x");
                while (len(polybius_table) >= 3) {
                    // debug("len %d", len(buffer_ptr_base));
                    putchar(cipher(polybius_table_ptr));
                }
            }
            prev_c = c;
        }
        else if (c == 9) {
            if (prev_c != 9 && prev_c != 32) {
                strappend(polybius_table_ptr, "x");
                while (len(polybius_table) >= 3) {
                    // debug("len %d", len(buffer_ptr_base));
                    putchar(cipher(polybius_table_ptr));
                }
            }
            prev_c = c;
        }
        else if (c == -1) {
            if (prev_c != 32 && prev_c != 9 && prev_c != 10) {
                strappend(polybius_table_ptr, "x");
                while (len(polybius_table) >= 3) {
                    // debug("len %d", len(buffer_ptr_base));
                    putchar(cipher(polybius_table_ptr));
                }
            }
            // debug("buffer_ptr: %s", buffer_ptr_base);
            return EXIT_SUCCESS;
        }
        else {
            // debug("you entered %d, this translates to %s", c, translate_to_morse_code(c));
            morse_code = (char*)translate_to_morse_code(c);
            prev_c = c;
            if (*morse_code) {
                debug("Input char: %c\tmorse code translation: %s", c, morse_code);
                while (len(polybius_table) >= 3) {
                    // debug("len %d", len(buffer_ptr_base));
                    putchar(cipher(polybius_table_ptr));
                }
                strappend(polybius_table_ptr, morse_code);
                while (len(polybius_table) >= 3) {
                    // debug("len %d", len(buffer_ptr_base));
                    putchar(cipher(polybius_table_ptr));
                }
                strappend(polybius_table_ptr, "x");
            }
            else {
                debug("BAD input! There is no translatable morse code for char %c", c);
                return EXIT_FAILURE;
            }
        }

    }
    return EXIT_FAILURE;
}


char translate_to_plain_text(const char *morse) {
    // debug("translating char %c to morse code\n", c);
    const char **morse_table_ptr = morse_table;
    char c = START_ASCII;
    while (*morse_table_ptr) {
        debug("looking for %s in morse table: %s", morse, *morse_table_ptr);
        if (scomp(morse, *morse_table_ptr)) break;
        c++;
        morse_table_ptr++;
    }
    return c;
}

char morse_decrypt() {
    char b = _morse_decrypt();
    if (*polybius_table) {
        // has x

        char xpos = str_has_char2(polybius_table, 'x');
        unsigned char blen = len(polybius_table);
        if (xpos == -1) {
            putchar(translate_to_plain_text(polybius_table));
        }
        else if (xpos == 0) {
            debug("%s", "STARTS WITH X");
            // check if next element is also x, else ignore x
            unsigned char next_char = *(polybius_table + 1);
            debug("NEXT CHAR: %c", next_char);
            if (next_char == 'x') {
                debug("%s", "FOLLOWED BY ANOTHER X");
                putchar(32);  // space
                // remove both x's from buffer
                shift_chars(polybius_table, 2);
                debug("polybius_table after shifthing: %s", polybius_table);
            } else {
                // remove x from buffer
                shift_chars(polybius_table, 1);
                debug("polybius_table after shifthing: %s", polybius_table);
            }
        }
        else if (xpos == (blen - 1)) {
            debug("%s", "ENDS WITH X");
            *(polybius_table + xpos) = 0;   // remove x from buffer to create substring
            // look up plaintext character
            blen = translate_to_plain_text(polybius_table);
            debug("translated to plaintext: %c", blen);
            shift_chars(polybius_table, xpos);
            debug("polybius_table after shifthing: %s", polybius_table);
            // put x back into buffer
            *polybius_table = 'x';
            debug("polybius_table after putting x back: %s", polybius_table);
            putchar(blen);
        }
        else {
            debug("%s", "X IS WITHIN");
            // get substring up to x
            *(polybius_table + xpos) = 0;
            // look up morse encoding
            blen = translate_to_plain_text(polybius_table);
            debug("translated to plaintext: %c", blen);
            putchar(blen);
            // restore original buffer
            *(polybius_table + xpos) = 'x';
            // clear already read chars
            shift_chars(polybius_table, xpos);
            debug("polybius_table after shifthing: %s", polybius_table);


            xpos = str_has_char2(polybius_table, 'x');
            blen = len(polybius_table);
            if (xpos == 0) {
                debug("%s", "STARTS WITH X");
                // check if next element is also x, else ignore x
                unsigned char next_char = *(polybius_table + 1);
                debug("NEXT CHAR: %c", next_char);
                if (next_char == 'x') {
                    debug("%s", "FOLLOWED BY ANOTHER X");
                    putchar(32);  // space
                    // remove both x's from buffer
                    shift_chars(polybius_table, 2);
                    debug("polybius_table after shifthing: %s", polybius_table);
                } else {
                    // remove x from buffer
                    shift_chars(polybius_table, 1);
                    debug("polybius_table after shifthing: %s", polybius_table);
                }
            }
            else if (xpos == (blen - 1)) {
                debug("%s", "ENDS WITH X");
                *(polybius_table + xpos) = 0;   // remove x from buffer to create substring
                // look up plaintext character
                blen = translate_to_plain_text(polybius_table);
                debug("translated to plaintext: %c", blen);
                shift_chars(polybius_table, xpos);
                debug("polybius_table after shifthing: %s", polybius_table);
                // put x back into buffer
                *polybius_table = 'x';
                debug("polybius_table after putting x back: %s", polybius_table);
                putchar(blen);
            }
        }

    }
    return b;
}

char _morse_decrypt() {
    // buffer for morse code translation
    char *buffer_ptr = polybius_table;
    unsigned char i = 0;
    const char *fm_key_ptr = fm_key;
    const char **fractionated_table_ptr = fractionated_table;
    char c = 0;

    // get input
    while (c != -1) {
        c = getchar();
        if (c == -1) {
            return EXIT_SUCCESS;
        }
        else if (c == 10) {
            putchar('\n');
        }
        else {
            while (*fm_key_ptr) {
                debug("cipher char: %c == fm_key_ptr: %c(code: %s)", c, *fm_key_ptr, *fractionated_table_ptr);
                // look up cipher letter morse code, (1 to 1 matching)
                if (c == *fm_key_ptr) {
                    // get the morse code translation
                    debug("buffer_ptr before: %s", buffer_ptr);
                    strappend(buffer_ptr, (char*)*fractionated_table_ptr);
                    debug("buffer_ptr after: %s", buffer_ptr);
                    fm_key_ptr = fm_key;
                    fractionated_table_ptr = fractionated_table;
                    // check if buffer has x as last char
                    // ciphered morse codes are always added in groups of 3
                    char xpos = str_has_char2(buffer_ptr, 'x');
                    unsigned char blen = len(buffer_ptr);
                    debug("position of x: %d", xpos);
                    debug("length of buffer: %d", blen);
                    if (xpos == -1) {
                        // debug("%s", "iefnoqienfoqinfi");

                    }
                    else if (xpos == 0) {
                        debug("%s", "STARTS WITH X");
                        // check if next element is also x, else ignore x
                        unsigned char next_char = *(buffer_ptr + 1);
                        debug("NEXT CHAR: %c", next_char);
                        if (next_char == 'x') {
                            debug("%s", "FOLLOWED BY ANOTHER X");
                            putchar(32);  // space
                            // remove both x's from buffer
                            shift_chars(buffer_ptr, 2);
                            debug("buffer_ptr after shifthing: %s", buffer_ptr);
                        } else {
                            // remove x from buffer
                            shift_chars(buffer_ptr, 1);
                            debug("buffer_ptr after shifthing: %s", buffer_ptr);
                        }
                    }
                    else if (xpos == (blen - 1)) {
                        debug("%s", "ENDS WITH X");
                        *(buffer_ptr + xpos) = 0;   // remove x from buffer to create substring
                        // look up plaintext character
                        blen = translate_to_plain_text(buffer_ptr);
                        debug("translated to plaintext: %c", blen);
                        shift_chars(buffer_ptr, xpos);
                        debug("buffer_ptr after shifthing: %s", buffer_ptr);
                        // put x back into buffer
                        *buffer_ptr = 'x';
                        debug("buffer_ptr after putting x back: %s", buffer_ptr);
                        putchar(blen);
                    }
                    else {
                        debug("%s", "X IS WITHIN");
                        // get substring up to x
                        *(buffer_ptr + xpos) = 0;
                        // look up morse encoding
                        blen = translate_to_plain_text(buffer_ptr);
                        debug("translated to plaintext: %c", blen);
                        putchar(blen);
                        // restore original buffer
                        *(buffer_ptr + xpos) = 'x';
                        // clear already read chars
                        shift_chars(buffer_ptr, xpos);
                        debug("buffer_ptr after shifthing: %s", buffer_ptr);


                        xpos = str_has_char2(buffer_ptr, 'x');
                        blen = len(buffer_ptr);
                        if (xpos == 0) {
                            debug("%s", "STARTS WITH X");
                            // check if next element is also x, else ignore x
                            unsigned char next_char = *(buffer_ptr + 1);
                            debug("NEXT CHAR: %c", next_char);
                            if (next_char == 'x') {
                                debug("%s", "FOLLOWED BY ANOTHER X");
                                putchar(32);  // space
                                // remove both x's from buffer
                                shift_chars(buffer_ptr, 2);
                                debug("buffer_ptr after shifthing: %s", buffer_ptr);
                            } else {
                                // remove x from buffer
                                shift_chars(buffer_ptr, 1);
                                debug("buffer_ptr after shifthing: %s", buffer_ptr);
                            }
                        }
                        else if (xpos == (blen - 1)) {
                            debug("%s", "ENDS WITH X");
                            *(buffer_ptr + xpos) = 0;   // remove x from buffer to create substring
                            // look up plaintext character
                            blen = translate_to_plain_text(buffer_ptr);
                            debug("translated to plaintext: %c", blen);
                            shift_chars(buffer_ptr, xpos);
                            debug("buffer_ptr after shifthing: %s", buffer_ptr);
                            // put x back into buffer
                            *buffer_ptr = 'x';
                            debug("buffer_ptr after putting x back: %s", buffer_ptr);
                            putchar(blen);
                        }
                    }
                    break;
                }
                fractionated_table_ptr++;
                i++;
                fm_key_ptr++;
            }
        }
        debug("current buffer state: %s, char %c", buffer_ptr, c);
    }

    return EXIT_SUCCESS;
}
