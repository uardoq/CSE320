#include "helpers.h"
#include "debug.h"

unsigned short len(const char *s) {
    unsigned short sum = 0;
    // iterate over bytes until null.
    while (*s++) {
        sum++;
    }
    return sum;
}

char strtonum(const char *s) {
    char sum = 0, dec = 1;
    unsigned short n = len(s);

    for (int i = 1; i < n; ++i) {
        dec *= 10;
    }

    while (*s) {
        // check bounds
        if (*s < 48 || *s > 57) {
            debug("ascii out of bounds: %d", *s);
            return -1;
        }
        // ascii to dec
        sum += (*s - 48) * dec;

        // debug("digit: %d", *s);
        // debug("current sum: %d", sum);

        s++;
        dec /= 10;
    }
    return sum;
}


unsigned char scomp(const char *str1, const char *str2) {
    if (len(str1) != len(str2)) {
        return 0;
    }
    while (*str1 != 0 && *str2 != 0) {
        if (*str1++ != *str2++) {
            return 0;
        }
    }
    return 1;
}


void setbit(unsigned short *b, char pos, char x) {
    // for x = 0, pos = 0, (1111 xor 0001) and 1001 = 1000
    if (x == 0) {
        *b &= (-1 ^ (1 << pos));
    }
    else if (x == 1) {
        *b |= (1 << pos);
    }
}

char hasdupes(const char *s) {
    unsigned short slen = len(s);
    for (int i = 0; i < slen; ++i) {
        for (int j = i + 1; j < slen; ++j) {
            // j("cmp %c == %c", *(s + i), *(s + j));
            if (*(s + i) == *(s + j)) {
                // debug("%c == %c duplicate match", *(s + i), *(s + j));
                return 1;
            }
        }
    }
    return 0;
}

char str_has_char2(char *str, char c) {
    char *p = str;
    char i = 0;
    while (*p) {
        // debug("%c == %c", *p, c);
        if (*p == c) return i;
        p++;
        i++;
    }
    return -1;

}

char str_has_char(char *str, char c) {
    char *p = str;
    while (*p) {
        // debug("%c == %c", *p, c);
        if (*p == c) return 1;
        p++;
    }
    return 0;
}

unsigned char get_next_char(const char *str, char c) {

    const char *ptr = str;
    char min = 127, diff;
    char nextc = *(ptr + 1);

    while (*ptr) {
        diff = *ptr - c;
        // debug("diff: %c - %c ", *ptr, c);
        if (diff > 0 && diff < min) {
            min = diff;
            nextc = *ptr;
        }
        ptr++;
    }
    // debug("input string: %s, prev char: %c, next char: %c\n", str, c, nextc);
    return nextc;
}

char numtostr(unsigned char n) {
    // debug("numtostr input: %d", n);
    if (n >= 0 && n <= 9) return n + 48;
    else if (n <= 15) return 65 + (n - 10);
    else return -1;
}

char asciitonum(unsigned char n) {
    // debug("asciitonum input: %d", n);
    if (n >= 48 && n <= 57) return n - 48;
    else if (n <= 70) return 10 + (n - 65);
    else return -1;
}

/**
 * Rearrages characters in <str> to the start of string array <arr> and
 * puts the rearranged string array into <rearr>
 * @param  str
 * @param  arr
 * @param  rearr
 * @return the last rearranged element index.
 */
unsigned char set_string_to_array_start(const char *str, const char *arr, char *rearr) {
    unsigned char e = 0;
    const char *arr_ptr = arr;
    const char *str_ptr = str;
    char *rearr_ptr = rearr;

    // rearrange str to be at the start of rearr.
    while (*str_ptr) {
        *rearr_ptr = *str_ptr;
        // debug("rearranged char: %c", *rearr_ptr);
        str_ptr++;
        rearr_ptr++;
        e++;
    }
    // put the rest of the arr in rearr;
    str_ptr = str;
    unsigned char current_char = 0, next_char = 0;
    // debug("%s\n", "adding the rest of the alphabet...");
    // starting with smallest char in ascending order
    while (*arr_ptr) {
        next_char = get_next_char(str, current_char);
        // if char positions match, don't add to table.
        // debug("testing if str and arr char position match: %c", next_char);
        if (*arr_ptr == next_char) {
            current_char = next_char;
            // debug("%c match\n", next_char);
            str_ptr++;
        }
        else {

            // otherwise add the character and update element position, e.
            // debug("adding arr char: %c\n", *arr_ptr);
            *rearr_ptr = *arr_ptr;
            rearr_ptr++;
            e++;
        }
        arr_ptr++;
    }

    return e;
}

/**
 * shifts left an amount of chars num_chars of a string.
 * @param buffer the base address of the string (base address does not get modified)
 */
void shift_chars(char *buffer, char num_chars) {
    char tail_length = len(buffer + num_chars);
    char i = 0;
    char *p = buffer;
    char *q = buffer + num_chars;

    // debug("buffer before: %s", buffer);
    if (tail_length < num_chars) {
        // case: tail is smaller than head of string.
        while (i < tail_length) {
            *p = *q;
            *q = 0;
            p++;
            q++;
            i++;
        }
        while (i < num_chars) {
            *p = 0;
            i++;
            p++;
        }
    }
    else {
        // case: tail is equal or larger than head of string.
        while (i < tail_length) {
            *p = *q;
            *q = 0;
            p++;
            q++;
            i++;
        }
    }
}

/**
 * [strappend description]
 * @param appendto [description] must be the base address of the string.
 * @param str      [description] must be the base address of the string.
 */
void strappend(char *appendto, char *str) {
    // debug("appendto: %s\tlength: %d", appendto, len(appendto));
    // debug("str: %s\tlength: %d", str, len(str));
    char *appendto_ptr = appendto;
    // move appendto to the position after the last character.
    while (*appendto_ptr) appendto_ptr++;
    // append the string at the end of appendto
    while (*str) {
        *appendto_ptr = *str;
        appendto_ptr++;
        str++;
    }

}