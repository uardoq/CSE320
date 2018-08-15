#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "hw1.h"
#include "helpers.h"
#include "const.h"

Test(hw1_tests_suite, key_validation_test) {

    // polybius_alphabet
    // fm_alphabet
    char *test_key = "HONEA";
    char rslt = valkey(test_key, fm_alphabet);
    char exp = 1;
    cr_assert_eq(rslt, exp, "in: %s, result: %d, expected: %d", test_key, exp, exp);
}

Test(hw1_tests_suite, key_dup_test) {
    char rslt = hasdupes("helo");
    char exp = 0;
    cr_assert_eq(rslt, exp, "rslt %d, exp %d", rslt, exp);
}
