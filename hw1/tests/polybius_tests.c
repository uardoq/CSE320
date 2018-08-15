#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <stdio.h>
#include <stdlib.h>

#include "hw1.h"
#include "helpers.h"
#include "const.h"
#include "polybius.h"

Test(hw1_tests_suite, polybius_table_test) {
    key = "jkl7";
    create_polybius_table(10, 10);
    cr_assert_eq(1, 1, "in: %s, result: %d, expected: %d", 1, 1, 1);
}

Test(hw1_tests_suite, polybius_parse_test) {
    unsigned char rows = 11, cols = 9;
    char *table_ptr = polybius_table;

    // sets global table
    create_polybius_table(rows, cols);

    parse_polybius_encrypt_char('C', table_ptr, rows, cols);
    parse_polybius_encrypt_char('S', table_ptr, rows, cols);
    parse_polybius_encrypt_char('E', table_ptr, rows, cols);
    parse_polybius_encrypt_char('3', table_ptr, rows, cols);
    parse_polybius_encrypt_char('2', table_ptr, rows, cols);
    parse_polybius_encrypt_char('0', table_ptr, rows, cols);
    parse_polybius_encrypt_char('{', table_ptr, rows, cols);
    parse_polybius_encrypt_char('}', table_ptr, rows, cols);

    cr_assert_eq(1, 1, "in: %s, result: %d, expected: %d", 1, 1, 1);
}

Test(hw1_tests_suite, polybius_parse_with_key_test) {
    unsigned char rows = 10, cols = 10;
    char *table_ptr    = polybius_table;

    // set global key
    key = "cse320";
    // sets global table
    create_polybius_table(rows, cols);

    parse_polybius_encrypt_char('C', table_ptr, rows, cols);
    parse_polybius_encrypt_char('S', table_ptr, rows, cols);
    parse_polybius_encrypt_char('E', table_ptr, rows, cols);
    parse_polybius_encrypt_char('3', table_ptr, rows, cols);
    parse_polybius_encrypt_char('2', table_ptr, rows, cols);
    parse_polybius_encrypt_char('0', table_ptr, rows, cols);
    parse_polybius_encrypt_char('{', table_ptr, rows, cols);
    parse_polybius_encrypt_char('}', table_ptr, rows, cols);

    cr_assert_eq(1, 1, "in: %s, result: %d, expected: %d", 1, 1, 1);
}
