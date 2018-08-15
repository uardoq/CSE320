#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "helpers.h"
#include "debug.h"

Test(helpers_test_suite, contains_char_test) {
    char *str = "helo";
    char c = 'o';
    char b = str_has_char(str, c);
    char exp = 1;
    cr_assert_eq(b, exp, "string: %s, char: %c, expected: %s", str, c, exp ? "true" : "false");
}

Test(helpers_test_suite, bitset_test) {
    // in:  0b 1000 1000 1000 1001 = 34953
    // out: 0b 1000 0000 1000 1001 = 32905
    unsigned short in = 34953, out = in, exp = 32905;
    char pos = 11;
    setbit(&out, pos, 0);   // set 11th bit to 0
    cr_assert_eq(out, exp, "in: %d, out: %d, expected: %d", in, out, exp);
}

Test(helpers_test_suite, str_to_num_test) {
    char *str = "1c";
    char out = strtonum(str);
    char exp = -1;
    cr_assert_eq(out, exp, "in: %s, out: %d, expected: %d", str, out, exp);
}

Test(helpers_test_suite, shift_chars_test) {
    long buffer = 0;
    char *buffer_ptr = (char*)&buffer;
    char *buffer_ptr_base = buffer_ptr;
    // input = "x.-.." shifting 3 --> outputs: ".."
    *buffer_ptr = 'x';
    buffer_ptr++;
    *buffer_ptr = '.';
    buffer_ptr++;
    *buffer_ptr = '-';
    buffer_ptr++;
    *buffer_ptr = '.';
    buffer_ptr++;
    *buffer_ptr = '.';

    char l = len(buffer_ptr_base);
    debug("shift_chars_test in: %s", buffer_ptr_base);
    shift_chars(buffer_ptr_base, 3);
    debug("shift_chars_test out: %s", buffer_ptr_base);

    // expect the base address to dereference to \0, the same goes for the rest of the junk chars.
    cr_assert_eq(len(buffer_ptr_base), l - 3, "in: %s, out: %s, expected: %d", buffer_ptr_base, buffer_ptr_base, 0);
}

Test(helpers_test_suite, str_append_test) {
    long buffer = 0;
    char *buffer_ptr = (char*)&buffer;
    char *buffer_ptr_base = buffer_ptr;
    *buffer_ptr = '-';
    buffer_ptr++;
    *buffer_ptr = '.';
    buffer_ptr++;
    *buffer_ptr = '-';
    buffer_ptr++;
    *buffer_ptr = 'x';

    char *appendme = "K";
    // char appendme = 'K';
    debug("append_str_test in: %s", buffer_ptr_base);
    strappend(buffer_ptr_base, appendme);
    debug("append_str_test in: %s", buffer_ptr_base);

    cr_assert_eq(scomp(buffer_ptr_base, "-.-xK"), 1, "in: %s, out: %s, expected: %d", buffer_ptr_base, buffer_ptr_base, 1);
}

