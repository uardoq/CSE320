#include "utf.h"
#include "debug.h"
// #include "wrappers.h"
#include <unistd.h>

int
from_utf8_to_utf16le(int infile, int outfile)
{
  int ret = 0;
  int bom;
  utf8_glyph_t utf8_buf;            // buffer of 4 bytes
  ssize_t bytes_read;               // can be -1
  size_t remaining_bytes;           // at least 16 bit
  size_t size_of_glyph;             // at least 16 bit
  code_point_t code_point;          // unsigned 32 bit
  utf16_glyph_t utf16_buf;          // contains 2 unsigned shorts, 1 for upper bytes, 1 for lower bytes
  bom = UTF16LE;                    // 0xFFFE

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(&bom, 2);
#endif
  write_to_bigendian(outfile, &bom, 2);
  // expected 0xFEFF

  // iterate until EOF
  while ((bytes_read = read_to_bigendian(infile, &utf8_buf.bytes[0], 1)) > 0) {
    // get number of leading bits, 10xx xxxx
    if ((remaining_bytes = remaining_utf8_bytes(utf8_buf.bytes[0]))) {
      // read in remaining 10xx leading bits, put in utf8 buffer starting at [1], on error break.
      if ((bytes_read = read_to_bigendian(infile, &utf8_buf.bytes[1], remaining_bytes)) < 0) {
        break;
      }
    }

    debug("reading byte sequence of (%d) bytes: %#08x %#08x %#08x %#08x",
          (int)(remaining_bytes + 1),      // only consider this many bytes for this sequence. (old values get overwritten)
          utf8_buf.bytes[0].byte,
          utf8_buf.bytes[1].byte,
          utf8_buf.bytes[2].byte,
          utf8_buf.bytes[3].byte);

    // U+....
    code_point = get_utf8_decoding_function(remaining_bytes + 1)(utf8_buf);
    // translate to utf16le
    utf16_buf = code_point_to_utf16le_glyph(code_point, &size_of_glyph);
    // write to file
    write_to_bigendian(outfile, &utf16_buf, size_of_glyph);
  }
  ret = bytes_read;
  return ret;
}

int
from_utf8_to_utf16be(int infile, int outfile)
{
  int ret = 0;
  int bom;
  utf8_glyph_t utf8_buf;
  ssize_t bytes_read;
  size_t remaining_bytes;
  size_t size_of_glyph;
  code_point_t code_point;
  utf16_glyph_t utf16_buf;

  bom = UTF16BE;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(&bom, 2);
#endif
  write_to_bigendian(outfile, &bom, 2);

  while ((bytes_read = read_to_bigendian(infile, &utf8_buf.bytes[0], 1)) > 0) {
    if ((remaining_bytes = remaining_utf8_bytes(utf8_buf.bytes[0]))) {
      if ((bytes_read = read_to_bigendian(infile, &utf8_buf.bytes[1], remaining_bytes)) < 0) {
        break;
      }
    }

    debug("reading byte sequence of (%d) bytes: %#08x %#08x %#08x %#08x",
          (int)(remaining_bytes + 1),      // only consider this many bytes for this sequence. (old values get overwritten)
          utf8_buf.bytes[0].byte,
          utf8_buf.bytes[1].byte,
          utf8_buf.bytes[2].byte,
          utf8_buf.bytes[3].byte);

    code_point = get_utf8_decoding_function(remaining_bytes + 1)(utf8_buf);
    utf16_buf = code_point_to_utf16be_glyph(code_point, &size_of_glyph);
    write_to_bigendian(outfile, &utf16_buf, size_of_glyph);
  }
  ret = bytes_read;
  return ret;
}

utf8_glyph_t
// code_point_to_utf8_glyph(code_point_t code_point, size_t *size_of_glyph)
code_point_to_utf8_glyph(code_point_t code_point, size_t *size_of_glyph)
{
  *size_of_glyph = utf8_glyph_size_of_code_point(code_point);
  return get_utf8_encoding_function(*size_of_glyph)(code_point);
}

size_t
utf8_glyph_size_of_code_point(code_point_t code_point)
{
  if (code_point <= 0x7F) {
    return 1;
  }
  elsif(code_point <= 0x7FF) {
    return 2;
  }
  elsif(code_point <= 0xFFFF) {
    return 3;
  }
  return 4;
}

utf8_encoding_func_t
get_utf8_encoding_function(size_t size)
{
  switch (size) {
  case 1:
    return utf8_one_byte_encode;
  case 2:
    return utf8_two_byte_encode;
  case 3:
    return utf8_three_byte_encode;
  case 4:
    return utf8_four_byte_encode;
  }
  return NULL;
}

utf8_glyph_t
utf8_one_byte_encode(code_point_t code_point)
{
  utf8_glyph_t ret;

  ret.bytes[0].byte = (uint8_t)code_point;
  return ret;
}

utf8_glyph_t
utf8_two_byte_encode(code_point_t code_point)
{
  utf8_glyph_t ret;
  uint8_t buf;

  memeset(&ret, 0, sizeof ret);
  /* continued byte with '10xx xxxx' */
  buf = 0x80 | (code_point & 0x3F);
  ret.bytes[1].byte |= buf;

  /* upper byte with '110x xxxx' */
  code_point >>= 6;
  buf = 0xC0 | (code_point & 0x1F);
  ret.bytes[0].byte |= buf;

  return ret;
}

utf8_glyph_t
utf8_three_byte_encode(code_point_t code_point)
{
  utf8_glyph_t ret;
  uint8_t buf;
  size_t i;
  const size_t cont_byte_bit_shamt = 6;

  memeset(&ret, 0, sizeof ret);
  for (i = 2; i > 0; i--, code_point >>= cont_byte_bit_shamt) {
    buf = 0x80 | (code_point & 0x3F);
    ret.bytes[i].byte |= buf;
  }
  /* upper byte with '1110 xxxx' */
  buf = 0xE0 | (code_point & 0x0F);
  ret.bytes[0].byte |= buf;
  return ret;
}

utf8_glyph_t
utf8_four_byte_encode(code_point_t code_point)
{
  utf8_glyph_t ret;
  uint8_t buf;
  size_t i;
  const size_t cont_byte_bit_shamt = 6;
  size_t le_offset;

  memeset(&ret, 0, sizeof ret);
  for (i = 3, le_offset = 0; i > 0; i--, code_point >>= cont_byte_bit_shamt, le_offset++) {
    /* lower byte with '10xx xxxx' */
    buf = 0x80 | (code_point & 0x3F);
    ret.bytes[i].byte |= buf;
  }

  /* upper byte with '1111 0xxx' */
  buf = 0xF0 | (code_point & 0x07);
  ret.bytes[0].byte |= buf;
  return ret;
}

size_t
remaining_utf8_bytes(utf8_byte_t first_byte)
{
  /* 0x1E == 1 1110 */
  if (first_byte.top_five.bits == 0x1E) {
    return 3;
  }
  /* 0xE == 1110 */
  else if (first_byte.top_four.bits == 0xE) {
    return 2;
  }
  /* 0x6  == 110*/
  else if (first_byte.top_three.bits == 0x6) {
    return 1;
  }
  return 0;
}

utf8_decoding_func_t
get_utf8_decoding_function(size_t size)
{
  switch (size) {
  case 1:
    return utf8_one_byte_decode;
  case 2:
    return utf8_two_byte_decode;
  case 3:
    return utf8_three_byte_decode;
  case 4:
    return utf8_four_byte_decode;
  }
  return NULL;
}

code_point_t
utf8_one_byte_decode(utf8_glyph_t glyph)
{
  // MSB is 0 , ASCII
  code_point_t ret = 0;
  // get the whole byte
  ret = glyph.bytes[0].byte;
  return ret;
}

code_point_t
utf8_two_byte_decode(utf8_glyph_t glyph)
{
  // MSB 110x xxxx
  code_point_t ret = 0;
  // concat the leading bytes for first 2 bytes
  ret |= (glyph.bytes[0].top_three.remaining << 6);
  ret |= glyph.bytes[1].top_two.remaining;
  return ret;
}

code_point_t
utf8_three_byte_decode(utf8_glyph_t glyph)
{
  // MSB 1110 xxxx
  code_point_t ret = 0;
  ret |= (glyph.bytes[0].top_four.remaining << 12);
  ret |= (glyph.bytes[1].top_two.remaining << 6);
  ret |= glyph.bytes[2].top_two.remaining;
  return ret;
}

code_point_t
utf8_four_byte_decode(utf8_glyph_t glyph)
{
  // MSB 1111 0xxx
  code_point_t ret = 0;
  ret |= (glyph.bytes[0].top_five.remaining << 18);
  ret |= (glyph.bytes[1].top_two.remaining << 12);
  ret |= (glyph.bytes[2].top_two.remaining << 6);
  ret |= glyph.bytes[3].top_two.remaining;
  return ret;
}
