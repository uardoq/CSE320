Originally, this program did not compile, had memory leaks (because some resources were not freed), and was missing implementation for various functions.
 

```
./bin/utf [-h] -e ENCODING INPUT_FILE OUTPUT_FILE 

Translates unicode files between utf-8, utf-16le, and utf-16be

Option arguments:

-h          Displays this usage menu.
-e ENCODING MANDATORY FLAG: Choose output format.
            Accepted values:
             - UTF16LE
             - UTF16BE
             - UTF8

Positional arguments:

INPUT_FILE  File to convert.
            Must contain a Byte Order Marking (BOM)

OUTPUT_FILE Output file
            Will contain a Byte Order Marking (BOM)
```
