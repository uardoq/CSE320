#ifndef HELPERS_H
#define HELPERS_H

#define ALPHABET_DQUOTE     34  // "
#define ALPHABET_AMP        38  // &
#define ALPHABET_SQUOTE     39  // '
#define ALPHABET_L_PAREN    40  // [
#define ALPHABET_R_PAREN    41  // ]
#define ALPHABET_DASH       45  // -
#define ALPHABET_DOT        46  // .
#define ALPHABET_SLASH      47  //
#define DECIMAL_L_MIN       48  // 0
#define DECIMAL_L_MAX       57  // 9
#define ALPHABET_COLON      58  // :
#define ALPHABET_L_MIN      65  // a
#define ALPHABET_L_MAX      90  // z
#define ALPHABET_L_BRACKET  91  // [
#define ALPHABET_BSLASH     92  //
#define ALPHABET_R_BRACKET  93  // ]
#define ALPHABET_BACKTICK   94  // `
#define ALPHABET_UNDERSCORE 95  // _
#define ALPHABET_U_MIN      97  // A
#define ALPHABET_U_MAX      122 // Z
#define ALPHABET_TILDE      127 // ~

// tokenize readline
char **get_tokens_from_rl(char *line, char* separator, char *pipe);

char *home_aware_prompt();

int validate_input(char *input, int stops[]);

int in_alphabet(char c);

int in_special(char c);

char **get_args_to_delim(char *input, char *delim);

#endif