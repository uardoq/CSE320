#include "utf.h"
#include "debug.h"
// #include "wrappers.h"
#include <sys/sendfile.h>
#include <unistd.h>

// define extern vars in utf.h
const char *STR_UTF16BE = "UTF16BE";
char *const STR_UTF16LE = "UTF16LE";
char const *STR_UTF8  = "UTF8";

convertion_func_t
get_encoding_function()
{
  encoding_from_to_t translate = (program_state->encoding_from - program_state->encoding_to);
  switch (translate) {
    case utf8_to_utf16le:
      return from_utf8_to_utf16le;
    case utf8_to_utf16be:
      return from_utf8_to_utf16be;
    case utf16le_to_utf16be:
      return from_utf16le_to_utf16be;
    case utf16be_to_utf16le:
      return from_utf16be_to_utf16le;
    case utf16be_to_utf8:
      return from_utf16be_to_utf8;
    case utf16le_to_utf8:
      return from_utf16le_to_utf8;
    case transcribe_file:
      return transcribe;
  }
  return NULL;
}

void
check_bom()
{
  int fd;                 // file desc
  ssize_t bytes_read;     // -1 or MAX_SSIZE_T
  format_t bom = 0;       // UTF16LE | UTF16BE | UTF8

  if (program_state->in_file == NULL) {
    error("%s\n", "In file not specified");
    exit(EXIT_FAILURE);
  }
  fd = Open(program_state->in_file, O_RDONLY);

  // good
  if ((bytes_read = read_to_bigendian(fd, &bom, 3)) < 3) {
    fprintf(stderr, "%s\n", "File contains invalid BOM or is empty beyond BOM");
    exit(EXIT_FAILURE);
  }
  debug("BOM: %X", bom);
  if (bom == UTF8)
  {
    info("Source BOM: %s", STR_UTF8);
    program_state->encoding_from = UTF8;
    program_state->bom_length = 3;          // 3 bytes
    close(fd);
    return;
  }

  // needed
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(&bom, 2);
#endif

  debug("BOM AFTER SWAP: %X", bom);
  if (LOWER_TWO_BYTES(bom) == UTF16LE) {
    info("Source BOM: %s", STR_UTF16LE);
    program_state->encoding_from = UTF16LE;
    program_state->bom_length = 2;
  }
  else if (LOWER_TWO_BYTES(bom) == UTF16BE)
  {
    info("Source BOM: %s", STR_UTF16BE);
    program_state->encoding_from = UTF16BE;
    program_state->bom_length = 2;
  }
  else
  {
    fprintf(stderr, "%s\n", "Unrecognized BOM");
    exit(EXIT_FAILURE);
  }
  close(fd);
}

int
transcribe(int infile, int outfile)
{
  int ret = 0;
  struct stat infile_stat;
  // fstat gets metadata about file, returns -1 on failure. puts data into infile_stat on success.
  if (fstat(infile, &infile_stat) < 0) {
    perror("Could not stat infile");
    exit(EXIT_FAILURE);
  }
  // copies entire contents of infile to outfile
  // on success, returns num of bytes written to outfile
  ret = sendfile(outfile, infile, NULL, infile_stat.st_size);
  return ret;
}

bool
is_upper_surrogate_pair(utf16_glyph_t glyph)
{
  return ((glyph.upper_bytes > 0xD800) && (glyph.upper_bytes < 0xDBFF));
}

bool
is_lower_surrogate_pair(utf16_glyph_t glyph)
{
  return ((glyph.lower_bytes > 0xDC00) && (glyph.lower_bytes < 0xDFFF));
}

code_point_t
utf16_glyph_to_code_point(utf16_glyph_t *glyph)
{
  code_point_t ret = 0;
  if (!is_upper_surrogate_pair(*glyph)) {
    ret = glyph->upper_bytes;
  }
  else {
    // ret = (((glyph->upper_bytes - 0xD800) << 100) |
    //       ((glyph->lower_bytes - 0xDC00) & 0x3FF)) +
    //       0x10000;
    ret = (((glyph->upper_bytes - 0xD800) << 10) |
           ((glyph->lower_bytes - 0xDC00) & 0x3FF)) +
          0x10000;
  }
  return ret;
}

bool
is_code_point_surrogate(code_point_t code_point)
{
  return (code_point >= 0x10000);
}
