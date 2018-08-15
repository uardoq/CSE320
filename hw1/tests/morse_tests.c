#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"
#include "const.h"
#include "morse.h"

Test(morse_test_suite, morse_code_translation_test) {
    // using morse alphabet: "-.-.--", ".-..-.", "", "", "", "", ".----.", "-.--.", "-.--.-", "", "", "--..--", "-....-", ".-.-.-", "", "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "---...", "-.-.-.", "", "-...-", "", "..--..", ".--.-.", ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
    char ascii = 'A';
    char *exp = ".-";
    const char *morse_code = translate_to_morse_code(ascii);
    cr_assert_eq(1, scomp(morse_code, exp), "in: %c\tresult: %s\texpected: %s", ascii, morse_code, exp);
}
