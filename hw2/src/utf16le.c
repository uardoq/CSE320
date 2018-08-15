#include "utf.h"
#include "debug.h"
// #include "wrappers.h"
#include <unistd.h>

int
from_utf16le_to_utf16be(int infile, int outfile)
{
  int bom;
  utf16_glyph_t buf;
  ssize_t bytes_read;
  size_t bytes_to_write;
  int ret = 0;

  bom = UTF16BE;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(&bom, 2);
#endif
  write_to_bigendian(outfile, &bom, 2);

  while ((bytes_read = read_to_bigendian(infile, &(buf.upper_bytes), 2)) > 0) {
    bytes_to_write = 2;
    reverse_bytes(&(buf.upper_bytes), 2);
    if (is_lower_surrogate_pair(buf)) {
      if ((bytes_read = read_to_bigendian(infile, &(buf.lower_bytes), 2)) < 0) {
        break;
      }
      reverse_bytes(&(buf.lower_bytes), 2);
      bytes_to_write += 2;
    }
    write_to_bigendian(outfile, &buf, bytes_to_write);
  }
  ret = bytes_read;
  return ret;
}

int
from_utf16le_to_utf8(int infile, int outfile)
{
  // read seq of 2 bytes or 4 bytes if surrogate
  // translate to code point
  // transform code point to UTF8
  // write this value to outfile

  int ret = 0;
  int bom;
  ssize_t bytes_read;
  size_t bytes_to_write;
  utf16_glyph_t utf16_buf;
  code_point_t code_point;
  utf8_glyph_t utf8_buf;

  // write bom for utf8
  bom = UTF8;
  // dont have to reverse byte when going from utfle to utf8
  write_to_bigendian(outfile, &bom, 3);

  // reading seq: 0x21 0x00 0xe9 0x00 0xe5 0x77 ...
  // 0x0021 0x00e9 0x77e5 ...
  while ((bytes_read = read_to_bigendian(infile, &(utf16_buf.upper_bytes), 2)) > 0) {
    bytes_to_write = 2;
    // check if surrogate ie 2 2 bytes
    if (is_upper_surrogate_pair(utf16_buf)) {
      // get second pair
      if ((bytes_read = read_to_bigendian(infile, &(utf16_buf.lower_bytes), 2)) < 0) {
        break;
      }
      bytes_to_write += 2;
    }
    code_point = utf16_glyph_to_code_point(&utf16_buf);
    utf8_buf = code_point_to_utf8_glyph(code_point, &bytes_to_write);
    // bytes_to_write updated to num of utf8 bytes to write
    write_to_bigendian(outfile, &utf8_buf, bytes_to_write);
  }

  return ret = bytes_read;
}

utf16_glyph_t
code_point_to_utf16le_glyph(code_point_t code_point, size_t *size_of_glyph)
{
  utf16_glyph_t ret;  // upper / lower bytes

  // set to 0
  memeset(&ret, 0, sizeof ret);
  // >= 10000
  if (is_code_point_surrogate(code_point)) {
    debug("code_point: U+%08x is surrogate", code_point);
    code_point -= 0x10000;
    ret.upper_bytes = (code_point >> 10) + 0xD800;
    ret.lower_bytes = (code_point & 0x3FF) + 0xDC00;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    reverse_bytes(&ret.upper_bytes, 2);
    reverse_bytes(&ret.lower_bytes, 2);
#endif
    *size_of_glyph = 4;
  }
  else {
    ret.upper_bytes |= code_point;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    reverse_bytes(&ret.upper_bytes, 2);
#endif
    *size_of_glyph = 2;
  }
  return ret;
}
