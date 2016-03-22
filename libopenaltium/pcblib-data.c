/*
 *  openaltium is a set of tools for opening Altium (TM) library files
 *  Copyright (C) 2011  Peter Clifton <pcjc2@cam.ac.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "content-parser.h"


static void
print_coord (int32_t coord)
{
  printf ("%.2fmm", (double)coord / 1000000. * 2.54);
}

static int
decode_name (file_content *content)
{
  char *string;

  printf ("Decoding name header\n");

  if ((string = content_get_length_multi_prefixed_string (content)) == NULL) return 0;
  printf ("  String is '%s'\n", string);

  g_free (string);
  return 1;
}

static int
decode_arc_record (file_content *content)
{
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t x, y, radius;
  double start_angle, end_angle;
  uint32_t dw1, dw2;

  printf ("Decoding arc record\n");

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  if (!content_get_uint16 (content, &w1)) return 0;
  if (!content_get_uint16 (content, &w2)) return 0;
  if (!content_get_uint16 (content, &w3)) return 0;
  printf ("  WORDS %i, %i, %i\n", w1, w2, w3);

  content_skip_bytes (content, 10);
  printf ("  Skipped 10 bytes\n");

  if (!content_get_int32 (content, &x)) return 0;
  if (!content_get_int32 (content, &y)) return 0;
  if (!content_get_int32 (content, &radius)) return 0;
  printf ("  Center location (");
  print_coord (x); printf (", ");
  print_coord (y); printf (") Radius?: ");
  print_coord (radius); printf ("\n");

  if (!content_get_double (content, &start_angle)) return 0;
  if (!content_get_double (content, &end_angle)) return 0;
  printf ("  Angle %f°-%f°\n", start_angle, end_angle);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  printf ("  Next dimension pair: ");
  print_coord (dw1);
  printf (", ");
  print_coord (dw2);
  printf ("\n");

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  return 1;
}

static int
decode_record_3 (file_content *content)
{
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t x, y, c1, c2, c3, c4, c5, c6, c7;
  uint32_t dw1, dw2;

  printf ("Decoding record of type 3 (unknown meaning)\n");

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i", byte);

  if (!content_get_uint16 (content, &w1)) return 0;
  if (!content_get_uint16 (content, &w2)) return 0;
  if (!content_get_uint16 (content, &w3)) return 0;
  printf ("  WORDS %i, %i, %i", w1, w2, w3);

  content_skip_bytes (content, 10);
  printf ("  Skipped 10 bytes\n");

  if (!content_get_int32 (content, &x)) return 0;
  if (!content_get_int32 (content, &y)) return 0;
  printf ("  Position (");
  print_coord (x); printf (", ");
  print_coord (y); printf (")\n");

  if (!content_get_int32 (content, &c1)) return 0;
  if (!content_get_int32 (content, &c2)) return 0;
  if (!content_get_int32 (content, &c3)) return 0;
  if (!content_get_int32 (content, &c4)) return 0;
  if (!content_get_int32 (content, &c5)) return 0;
  if (!content_get_int32 (content, &c6)) return 0;
  if (!content_get_int32 (content, &c7)) return 0;
  printf ("  c1: ");
  print_coord (c1); printf (" c2: ");
  print_coord (c2); printf (" c3: ");
  print_coord (c3); printf (" c4: ");
  print_coord (c4); printf (" c5: ");
  print_coord (c5); printf (" c6: ");
  print_coord (c6); printf (" c7: ");
  print_coord (c7); printf ("\n");

  if (!content_get_int32 (content, &c1)) return 0;
  if (!content_get_int32 (content, &c2)) return 0;
  if (!content_get_int32 (content, &c3)) return 0;
  if (!content_get_int32 (content, &c4)) return 0;
  if (!content_get_int32 (content, &c5)) return 0;
  if (!content_get_int32 (content, &c6)) return 0;
  if (!content_get_int32 (content, &c7)) return 0;
  printf ("  c1: ");
  print_coord (c1); printf (" c2: ");
  print_coord (c2); printf (" c3: ");
  print_coord (c3); printf (" c4: ");
  print_coord (c4); printf (" c5: ");
  print_coord (c5); printf (" c6: ");
  print_coord (c6); printf (" c7: ");
  print_coord (c7); printf ("\n");

  if (!content_get_int32 (content, &c1)) return 0;
  if (!content_get_int32 (content, &c2)) return 0;
  if (!content_get_int32 (content, &c3)) return 0;
  if (!content_get_int32 (content, &c4)) return 0;
  if (!content_get_int32 (content, &c5)) return 0;
  if (!content_get_int32 (content, &c6)) return 0;
  if (!content_get_int32 (content, &c7)) return 0;
  printf ("  c1: ");
  print_coord (c1); printf (" c2: ");
  print_coord (c2); printf (" c3: ");
  print_coord (c3); printf (" c4: ");
  print_coord (c4); printf (" c5: ");
  print_coord (c5); printf (" c6: ");
  print_coord (c6); printf (" c7: ");
  print_coord (c7); printf ("\n");

  if (!content_get_int32 (content, &c1)) return 0;
  if (!content_get_int32 (content, &c2)) return 0;
  if (!content_get_int32 (content, &c3)) return 0;
  if (!content_get_int32 (content, &c4)) return 0;
  if (!content_get_int32 (content, &c5)) return 0;
  if (!content_get_int32 (content, &c6)) return 0;
  if (!content_get_int32 (content, &c7)) return 0;
  printf ("  c1: ");
  print_coord (c1); printf (" c2: ");
  print_coord (c2); printf (" c3: ");
  print_coord (c3); printf (" c4: ");
  print_coord (c4); printf (" c5: ");
  print_coord (c5); printf (" c6: ");
  print_coord (c6); printf (" c7: ");
  print_coord (c7); printf ("\n");

  if (!content_get_int32 (content, &c1)) return 0;
  if (!content_get_int32 (content, &c2)) return 0;
  if (!content_get_int32 (content, &c3)) return 0;
  if (!content_get_int32 (content, &c4)) return 0;
  if (!content_get_int32 (content, &c5)) return 0;
  if (!content_get_int32 (content, &c6)) return 0;
  if (!content_get_int32 (content, &c7)) return 0;
  printf ("  c1: ");
  print_coord (c1); printf (" c2: ");
  print_coord (c2); printf (" c3: ");
  print_coord (c3); printf (" c4: ");
  print_coord (c4); printf (" c5: ");
  print_coord (c5); printf (" c6: ");
  print_coord (c6); printf (" c7: ");
  print_coord (c7); printf ("\n");

  if (!content_get_int32 (content, &c1)) return 0;
  if (!content_get_int32 (content, &c2)) return 0;
  if (!content_get_int32 (content, &c3)) return 0;
  if (!content_get_int32 (content, &c4)) return 0;
  if (!content_get_int32 (content, &c5)) return 0;
  if (!content_get_int32 (content, &c6)) return 0;
  if (!content_get_int32 (content, &c7)) return 0;
  printf ("  c1: ");
  print_coord (c1); printf (" c2: ");
  print_coord (c2); printf (" c3: ");
  print_coord (c3); printf (" c4: ");
  print_coord (c4); printf (" c5: ");
  print_coord (c5); printf (" c6: ");
  print_coord (c6); printf (" c7: ");
  print_coord (c7); printf ("\n");

  if (!content_get_int32 (content, &c1)) return 0;
  if (!content_get_int32 (content, &c2)) return 0;
  if (!content_get_int32 (content, &c3)) return 0;
  printf ("  c1: ");
  print_coord (c1); printf (" c2: ");
  print_coord (c2); printf (" c3: ");
  print_coord (c3); printf ("\n");

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

  return 1;
}

static int
decode_silkline (file_content *content)
{
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t x1, y1, x2, y2, width;
  uint32_t dw1, dw2;

  printf ("Decoding silkline\n");

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i", byte);

  if (!content_get_uint16 (content, &w1)) return 0;
  if (!content_get_uint16 (content, &w2)) return 0;
  if (!content_get_uint16 (content, &w3)) return 0;
  printf ("  WORDS %i, %i, %i", w1, w2, w3);

  content_skip_bytes (content, 10);
  printf ("  Skipped 10 bytes\n");

  if (!content_get_int32 (content, &x1)) return 0;
  if (!content_get_int32 (content, &y1)) return 0;
  if (!content_get_int32 (content, &x2)) return 0;
  if (!content_get_int32 (content, &y2)) return 0;
  if (!content_get_int32 (content, &width)) return 0;
  printf ("  Silk line (");
  print_coord (x1); printf (", ");
  print_coord (y1); printf (")-(");
  print_coord (x2); printf (", ");
  print_coord (y2); printf (") Width: ");
  print_coord (width); printf ("\n");

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  printf ("  DWORD %i, %i\n", dw1, dw2);

  return 1;
}

static int
decode_text_record (file_content *content)
{
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t x, y, something;
  uint32_t dw1, dw2, dw3, dw4, dw5;
  char *string;

  printf ("Decoding text record\n");

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i", byte);

  if (!content_get_uint16 (content, &w1)) return 0;
  if (!content_get_uint16 (content, &w2)) return 0;
  if (!content_get_uint16 (content, &w3)) return 0;
  printf ("  WORDS %i, %i, %i", w1, w2, w3);

  content_skip_bytes (content, 10);
  printf ("  Skipped 10 bytes\n");

  if (!content_get_int32 (content, &x)) return 0;
  if (!content_get_int32 (content, &y)) return 0;
  if (!content_get_int32 (content, &something)) return 0;
  printf ("  Text position (");
  print_coord (x); printf (", ");
  print_coord (y); printf (") Height: ");
  print_coord (something); printf ("\n");

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  if (!content_get_uint32 (content, &dw3)) return 0;
  if (!content_get_uint32 (content, &dw4)) return 0;
  if (!content_get_uint32 (content, &dw5)) return 0;
  printf ("  DWORDS %i, %i, %i, %i, %i\n", dw1, dw2, dw3, dw4, dw5);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  string = content_get_n_wchars (content, 32);
  if (string == NULL) return 0;
  printf ("  Font is %s\n", string);
  g_free (string);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;  /* Text object number or pin association? */
  if (!content_get_uint32 (content, &dw3)) return 0;
  printf ("  DWORDS %i, %i, %i\n", dw1, dw2, dw3);

  if ((string = content_get_length_multi_prefixed_string (content)) == NULL) return 0;
  printf ("  Text is '%s'\n", string);
  g_free (string);

  return 1;
}

static int
decode_record_6 (file_content *content)
{
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t x, y, something;
  uint32_t dw1, dw2;

  printf ("Decoding record type 6 (unknown meaning, similar to unknown record 1)\n");

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  if (!content_get_uint16 (content, &w1)) return 0;
  if (!content_get_uint16 (content, &w2)) return 0;
  if (!content_get_uint16 (content, &w3)) return 0;
  printf ("  WORDS %i, %i, %i\n", w1, w2, w3);

  content_skip_bytes (content, 10);
  printf ("  Skipped 10 bytes\n");

  if (!content_get_int32 (content, &x)) return 0;
  if (!content_get_int32 (content, &y)) return 0;
  if (!content_get_int32 (content, &something)) return 0;
  printf ("  Coordinates? (");
  print_coord (x); printf (", ");
  print_coord (y); printf (") Something: ");
  print_coord (something); printf ("\n");

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  printf ("  DWORDS %i, %i\n", dw1, dw2);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  printf ("  DWORDS %i, %i\n", dw1, dw2);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  return 1;
}

static int
decode_polygon_record (file_content *content)
{
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t something;
  char *attributes;
  uint32_t count;
  int i;

  printf ("Decoding polygon\n");

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i", byte);

  if (!content_get_uint16 (content, &w1)) return 0;
  if (!content_get_uint16 (content, &w2)) return 0;
  if (!content_get_uint16 (content, &w3)) return 0;
  printf ("  WORDS %i, %i, %i", w1, w2, w3);

  content_skip_bytes (content, 10);
  printf ("  Skipped 10 bytes\n");

  if (!content_get_int32 (content, &something)) return 0;
  printf ("  Something: ");
  print_coord (something); printf ("\n");

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i", byte);

  attributes = content_get_length_dword_prefixed_string (content);
  if (attributes == NULL)
    return 0;
  printf ("  Polygon attributes: %s\n", attributes);
  g_free (attributes);

  if (!content_get_uint32 (content, &count)) return 0;

  printf ("  Polygon outline: ");

  for (i = 0; i < count; i++) {
    double x, y;
    if (!content_get_double (content, &x)) return 0;
    if (!content_get_double (content, &y)) return 0;
    printf ("("); print_coord (x);
    printf (","); print_coord (y);  printf (")");
    if (i + 1 < count)
      printf ("-");
  }
  printf ("\n");

  return 1;
}

static int
decode_model_record (file_content *content)
{
  uint8_t byte;
  uint32_t record_length;
  uint16_t w1, w2, w3;
  int32_t something;
  char *attributes;
  uint32_t count;
  int i;

  printf ("Decoding model record\n");

  if (!content_get_uint32 (content, &record_length)) return 0;
  printf ("  Record length is %i", record_length);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i", byte);

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i", w1);

  content_skip_bytes (content, 10);
  printf ("  Skipped 10 bytes\n");

  if (!content_get_int32 (content, &something)) return 0;
  printf ("  Something: ");
  print_coord (something); printf ("\n");

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i", byte);

  attributes = content_get_length_dword_prefixed_string (content);
  if (attributes == NULL)
    return 0;
  printf ("  Model attributes: %s\n", attributes);
  g_free (attributes);

  if (!content_get_uint32 (content, &count)) return 0;

  printf ("  Model outline: ");

  for (i = 0; i < count; i++) {
    double x, y;
    if (!content_get_double (content, &x)) return 0;
    if (!content_get_double (content, &y)) return 0;
    printf ("("); print_coord (x);
    printf (","); print_coord (y);  printf (")");
    if (i + 1 < count)
      printf ("-");
  }
  printf ("\n");

  return 1;
}

static int
decode_record_15 (file_content *content)
{
  uint8_t byte;
  uint32_t dw1, dw2;

  printf ("Decoding record type 15 (unknown meaning)\n");

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i", byte);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  printf ("  DWORDS %i, %i\n", dw1, dw2);

  return 1;
}

static int
decode_pin_record (file_content *content)
{
  uint8_t b1, b2, b3;
  uint16_t w1, w2, w3;
  int32_t x, y, c1, c2, c3, c4, c5, c6, c7;
  uint32_t dw1, dw2, dw3, dw4, dw5;
  char *string;
  int extra_end_dword = 0;

  printf ("Decoding pin record\n");

  if ((string = content_get_length_multi_prefixed_string (content)) == NULL) return 0;
  printf ("  Pin '%s'\n", string);
  g_free (string);

  if (!content_get_byte (content, &b1)) return 0;
  printf ("  BYTE %i\n", b1);
  if (!content_get_uint32 (content, &dw1)) return 0;
  printf ("  DWORD %i\n", dw1);

  if ((string = content_get_length_multi_prefixed_string (content)) == NULL) return 0;
  printf ("  Magic string '%s'\n", string);
  g_free (string);

  if (!content_get_uint32 (content, &dw1)) return 0;
  printf ("  DWORD %i\n", dw1);

  if (!content_get_byte (content, &b1)) return 0;
  if (!content_get_byte (content, &b2)) return 0;  /* Some kind of length coding? */
  printf ("  BYTES %i, %i\n", b1, b2);
  if (b2 == 114)
    extra_end_dword = 1;

  if (!content_get_uint16 (content, &w1)) return 0;
  if (!content_get_uint16 (content, &w2)) return 0;
  if (!content_get_uint16 (content, &w3)) return 0;
  printf ("  WORDS %i %i %i\n", w1, w2, w3);

  content_skip_bytes (content, 10);
  printf ("  Skipped 10 bytes\n");

  if (!content_get_int32 (content, &x)) return 0;
  if (!content_get_int32 (content, &y)) return 0;
  printf ("  Pin position (");
  print_coord (x); printf (", ");
  print_coord (y); printf (")\n");

  if (!content_get_int32 (content, &c1)) return 0;
  if (!content_get_int32 (content, &c2)) return 0;
  if (!content_get_int32 (content, &c3)) return 0;
  if (!content_get_int32 (content, &c4)) return 0;
  if (!content_get_int32 (content, &c5)) return 0;
  if (!content_get_int32 (content, &c6)) return 0;
  if (!content_get_int32 (content, &c7)) return 0;
  printf ("  c1: ");
  print_coord (c1); printf (" c2: ");
  print_coord (c2); printf (" c3: ");
  print_coord (c3); printf (" c4: ");
  print_coord (c4); printf (" c5: ");
  print_coord (c5); printf (" c6: ");
  print_coord (c6); printf (" c7: ");
  print_coord (c7); printf ("\n");

  if (!content_get_byte (content, &b1)) return 0;
  if (!content_get_byte (content, &b2)) return 0;
  if (!content_get_byte (content, &b3)) return 0;
  printf ("  BYTES %i %i %i\n", b1, b2, b3);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  if (!content_get_uint32 (content, &dw3)) return 0;
  if (!content_get_uint32 (content, &dw4)) return 0;
  if (!content_get_uint32 (content, &dw5)) return 0;
  printf ("  DWORDS %i, %i, %i, %i, %i\n", dw1, dw2, dw3, dw4, dw5);

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  if (!content_get_uint32 (content, &dw3)) return 0;
  if (!content_get_uint32 (content, &dw4)) return 0;
  if (!content_get_uint32 (content, &dw5)) return 0;
  printf ("  DWORDS %i, %i, %i, %i, %i\n", dw1, dw2, dw3, dw4, dw5);
  printf ("  (as coords: ");
  print_coord (dw1); printf (", ");
  print_coord (dw2); printf (", ");
  print_coord (dw3); printf (", ");
  print_coord (dw4); printf (", ");
  print_coord (dw5); printf (")\n");

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  if (!content_get_uint32 (content, &dw3)) return 0;
  if (!content_get_uint32 (content, &dw4)) return 0;
  if (!content_get_uint32 (content, &dw5)) return 0;
  printf ("  DWORDS %i, %i, %i, %i, %i\n", dw1, dw2, dw3, dw4, dw5);

  if (!extra_end_dword)
    return 1;

  if (!content_get_uint32 (content, &dw1)) return 0;
  printf ("  EXTRA END DWORD %i\n", dw1);

  return 1;
}

void
decode_data (file_content *content, int expected_sections)
{
  uint8_t byte;
  uint32_t length;
  int section_no = 0;

  printf ("Decoding data stream\n");

  /* File starts with a footprint name header */
  if (!decode_name (content))
    goto error;

  while (section_no < expected_sections && content->cursor < content->length) {

    if (!content_get_byte (content, &byte))
      goto error;

    switch (byte) {

      case 1:
        if (!decode_arc_record (content))
          goto error;
        break;

      case 2: /* Pad object? */
        if (!decode_pin_record (content))
          goto error;
        break;

      case 3:
        if (!decode_record_3 (content))
          goto error;
        break;

      case 4:
        if (!decode_silkline (content))
          goto error;
        break;

      case 5:
        if (!decode_text_record (content))
          goto error;
        break;

      case 6:
        if (!decode_record_6 (content))
          goto error;
        break;

      case 11:
        if (!decode_polygon_record (content))
          goto error;
        break;

      case 12:
        if (!decode_model_record (content))
          goto error;
        break;

      case 15: /* FromTo object? */
        if (!decode_record_15 (content))
          goto error;
        break;

      //case 0: /* Track line segment? */
      case 8: /* Net object? */
      case 9: /* Component object? */
      default:
        fprintf (stderr, "Unknown section header %i at position 0x%x\n", byte,
                 content->cursor - 1);
        goto error;
    }

    section_no ++;
  }

  return;

error:
  fprintf (stderr, "Oops\n");
  return;
}
