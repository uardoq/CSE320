#ifndef HELPERS_H
#define HELPERS_H
#endif

#ifndef _H
#define _H "-h"
#endif
#ifndef _P
#define _P "-p"
#endif
#ifndef _F
#define _F "-f"
#endif
#ifndef _E
#define _E "-e"
#endif
#ifndef _D
#define _D "-d"
#endif
#ifndef _K
#define _K "-k"
#endif
#ifndef _R
#define _R "-r"
#endif
#ifndef _C
#define _C "-c"
#endif

#ifndef MIN_AS
#define MIN_AS 3
#endif

#ifndef START_ASCII
#define START_ASCII 33
#endif
#ifndef END_ASCII
#define END_ASCII 122
#endif

/**
 * returns 0 if diff strings
 */
unsigned char scomp(const char *str1, const char *str2);

/**
 * returns num of chararacters in a string.
 */
unsigned short len(const char *s);

/**
 * set bit at pos to x
 */
void setbit(unsigned short *b, char pos, char x);

/**
 * str to number
 */
char strtonum(const char *s);

char hasdupes(const char *s);

/**
 * returns yes or no
 * @param  str [description]
 * @param  c   [description]
 * @return     [description]
 */
char str_has_char(char *str, char c);

/**
 * returns position of the char
 * @param  str [description]
 * @param  c   [description]
 * @return     [description]
 */
char str_has_char2(char *str, char c);

char sort_str(char *str);

unsigned char get_next_char(const char *str, char c);

char numtostr(unsigned char n);

char asciitonum(unsigned char n);

unsigned char set_string_to_array_start(const char *str, const char *arr, char *rearr);

void shift_chars(char *str, char num_chars);

void strappend(char *appendto, char *str);

