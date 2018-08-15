#include <stdlib.h>
#include <stdio.h>
#include "polybius.h"
#include "const.h"
#include "helpers.h"

char polybius_decrypt(unsigned char rows, unsigned char cols) {
    char r = 0, c = 0;
    while (r != -1) {
        r = getchar();
        if (r == 32) {
            // space
            putchar(' ');
        } else if (r == 9) {
            // tab
            putchar('\t');
        } else if (r == 10) {
            // new line
            putchar('\n');
        } else if (r == -1) {
            // EOF
            return EXIT_SUCCESS;
        }
        // input assumed to always be valid ascii as per doc.
        else {
            c = getchar();
            // debug("r: %c, c: %c", r, c);
            parse_polybius_decrypt_char(asciitonum(r), asciitonum(c), polybius_table, rows, cols);
        }
    }
    return EXIT_SUCCESS;
}

char polybius_encrypt(unsigned char rows, unsigned char cols) {
    // get user input
    int c = 0;
    while (c != -1) {
        c = getchar();
        // check if valid input
        if (c == 32) {
            // space
            putchar(' ');
        } else if (c == 9) {
            // tab
            putchar('\t');
        } else if (c == 10) {
            // new line
            putchar('\n');
        } else if (c == -1) {
            // EOF
            return EXIT_SUCCESS;
        } else if (c < 33 || c > 126) {
            debug("%s", "\nBAD input! exiting...\n");
            return EXIT_FAILURE;
        }
        else {
            parse_polybius_encrypt_char(c, polybius_table, rows, cols);
        }
    }
    return EXIT_SUCCESS;
}

void parse_polybius_decrypt_char(char row, char col, char *parsed_table, char rows, char cols) {
    // debug("row: %d, col: %d, cols: %d", row, col, cols);
    unsigned char index = col + cols * row;
    // debug("calculated index: %d", index);
    putchar(*(parsed_table + index));
}
void parse_polybius_encrypt_char(char character, char *parsed_table, char rows, char cols) {
    unsigned char i = 0;
    unsigned char row_index = 0, col_index = 0;
    char *p = parsed_table;
    // iterate over string, if match found, i has its linear index.
    while (*p) {
        // debug("%c == %c", *p, character);
        if (*p == character) break;
        p++;
        i++;
    }
    // use index to calculate the row and col in table.
    row_index = i / cols;               // calc character row index
    col_index = i - cols * row_index;   // calc character col index
    putchar(numtostr(row_index));
    putchar(numtostr(col_index));
    // debug("char: %c, (%d,%d)", character, row_index, col_index);
}

void create_polybius_table(unsigned char rows, unsigned char cols) {
    unsigned char rc    = rows * cols - 1;
    unsigned char e     = 0;                    // keep track of element index
    char *table_ptr     = polybius_table;
    char *alphabet_ptr  = polybius_alphabet;
    const char *key_ptr = key;
    if (key) {
        debug("creating polybius table with rows: %d, cols: %d, and key: %s", rows, cols, key);
        // put key starting at 0,0 of table
        while (*key_ptr) {
            *table_ptr = *key_ptr;
            // debug("table: %c", *table_ptr);
            key_ptr++;
            table_ptr++;
            e++;
        }
        // put the rest of the alphabet in table
        key_ptr = key;          // restore base address
        char current_char = 0, next_char = 0;
        // debug("%s\n", "adding the rest of the alphabet...");
        // starting with smallest char in ascending order
        while (*alphabet_ptr) {
            next_char = get_next_char(key, current_char);   // get next smallest char in key.
            // if char positions match, don't add to table.
            // debug("testing if key and alphabet char position match: %c", next_char);
            if (*alphabet_ptr == next_char) {
                current_char  = next_char;
                // debug("%c match\n", next_char);
                key_ptr++;
            }
            else {
                // otherwise add the character and update element position, e.
                // debug("adding alphabet letter: %c\n", *alphabet_ptr);
                *table_ptr = *alphabet_ptr;
                table_ptr++;
                e++;
            }
            alphabet_ptr++;
        }

    }
    else {
        // no key specfied, just copy the alphabet onto ptr
        debug("creating polybius table with rows: %d, cols: %d, and no key", rows, cols);
        while (*alphabet_ptr) {
            // debug("adding alphabet letter: %c\n", *alphabet_ptr);
            *table_ptr = *alphabet_ptr;
            table_ptr++;
            alphabet_ptr++;
            e++;
        }
    }

    // use last position of e to fill the remaining cells with \0
    for (; e <= rc; ++e) {
        *table_ptr = 0;
        table_ptr++;
    }

    // debug("total number of elements: %d", e);
    // table_ptr = polybius_table;
    // print table elements
    // while (*table_ptr) {
    //     debug("table: %c", *table_ptr);
    //     table_ptr++;
    // }

    debug("table string: %s", polybius_table);
}