#ifndef POLYBIUS_H
#define POLYBIUS_H

#include "debug.h"

#endif

char polybius_encrypt(unsigned char rows, unsigned char cols);

char polybius_decrypt(unsigned char rows, unsigned char cols);

/**
 * create polybius table used for encryption and decryption
 * uses key(if given) and polybius_table in const.h
 */
void create_polybius_table(unsigned char rows, unsigned char cols);

void parse_polybius_encrypt_char(char character, char *parsed_table, char rows, char cols);

void parse_polybius_decrypt_char(char row, char col, char *parsed_table, char rows, char cols);