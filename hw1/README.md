This command line program is used to encrypt and decrypt text input in 2 different types of encryption: Polybius cipher or Fractionated Morse cipher. 

```
USAGE: ./bin/hw1 [-h] -p|-f -e|-d [-k KEY] [-r ROWS] [-c COLUMNS]
    -p       Polybius Cipher
             Additional parameters: [-e | -d] [-i INPUT_FILE] [-o OUTPUT_FILE] [-k KEY] [-r ROWS] [-c COLUMNS]
                 -e           Encrypt using the Polybius cipher.
                 -d           Decrypt using the Polybius cipher.
                 -k           KEY is the key to be used in the cipher.
                              It must have no repeated characters and each character in the key
                              must be in the alphabet in `const.c`.
                 -r           ROWS is the number of rows to be used in the Polybius cipher table.
                              Must be between 10 and 16, inclusive. Defaults to 10
                 -c           COLUMNS is the number of columns to be used in the Polybius cipher table.
                              Must be between 10 and 16, inclusive. Defaults to 10

    -f       Fractionated Morse Cipher
             Additional parameters: [-e | -d] [-i INPUT_FILE] [-o OUTPUT_FILE] [-k KEY]
                 -e           Encrypt using the Fractionated Morse cipher.
                 -d           Decrypt using the Fractionated Morse cipher.
                 -k           KEY is the key to be used in the cipher.
                              It must have no repeated characters and each character in the key
                              must be in the alphabet in `const.c`.

    -h       Display this help menu.
```
