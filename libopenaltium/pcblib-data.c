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
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <glib.h>

typedef struct {
  char *data;
  unsigned int length;
  unsigned int cursor;
} file_content;

static void
print_coord (int32_t coord)
{
  printf ("%.2fmm", (double)coord / 1000000. * 2.54);
}

static int
content_check_available (file_content *content, unsigned int length)
{
  return (content->cursor + length > content->length) ? 0 : 1;
}

/* FIXME: These need to read from the file as little endian, the
 *        implementation only works in Little Endian computers.
 */
#define CONTENT_GET_TYPE(name, type) \
static int \
content_get_##name (file_content *content, type *data) \
{ \
  g_return_val_if_fail (content_check_available (content, sizeof (type)), 0); \
  *data = *(type*)&content->data[content->cursor]; \
  content->cursor += sizeof (type); \
  return 1; \
}

CONTENT_GET_TYPE(uint32, uint32_t)
CONTENT_GET_TYPE(int32, int32_t)
//CONTENT_GET_TYPE(int16, int16_t)
CONTENT_GET_TYPE(uint16, uint16_t)
CONTENT_GET_TYPE(byte, uint8_t)
CONTENT_GET_TYPE(double, double)

static char *
content_get_n_chars (file_content *content, unsigned int n_chars)
{
  char *data;
  g_return_val_if_fail (content_check_available (content, n_chars), NULL);
  data = g_strndup (&content->data[content->cursor], n_chars);
  content->cursor += n_chars;
  return data;
}

static char *
content_get_n_wchars (file_content *content, unsigned int n_chars)
{
  char *data;
  g_return_val_if_fail (content_check_available (content, 2 * n_chars), NULL);
  /* FIXME: Add error handling for this next call */
  data = g_utf16_to_utf8 ((gunichar2 *)&content->data[content->cursor], n_chars, NULL, NULL, NULL);
  content->cursor += 2 * n_chars;
  return data;
}

static int
content_skip_bytes (file_content *content, unsigned int n_bytes)
{
  g_return_val_if_fail (content_check_available (content, n_bytes), 0);
  content->cursor += n_bytes;
  return 1;
}

static char *
content_get_string (file_content *content)
{
  uint32_t txt_block_length;
  uint8_t txt_length;
  char *string;

  /* Text block skip length */
  if (!content_get_uint32 (content, &txt_block_length))
    return NULL;

  if (!content_get_byte (content, &txt_length))
    return NULL;

  g_return_val_if_fail (txt_block_length == 1 + txt_length, NULL);

  string = content_get_n_chars (content, txt_length);
  return string;
}

static int
decode_name (file_content *content)
{
  char *string;

  printf ("Decoding name header\n");

  if ((string = content_get_string (content)) == NULL) return 0;
  printf ("  String is '%s'\n", string);

  g_free (string);
  return 1;
}

static int
decode_attribute_record (file_content *content)
{
  uint32_t length;
  uint32_t count;
  double dbl1, dbl2;
  int i;
  char *string;

  printf ("Decoding attribute record\n");

  /* Text block length */
  if (!content_get_uint32 (content, &length))
    return 0;

  string = content_get_n_chars (content, length);
  if (string == NULL)
    return 0;
  printf ("  Attribute string: %s\n", string);
  g_free (string);

  /* XXX: Not sure if this is part of this section or something separate...
   *      wildly guessing at the significance of the presumed count DWORD
   */
  if (!content_get_uint32 (content, &count)) return 0;

  for (i = 0; i < count; i++) {
    if (!content_get_double (content, &dbl1)) return 0;
    if (!content_get_double (content, &dbl2)) return 0;
    printf ("  Count: %i  DOUBLES %f, %f\n", i, dbl1, dbl2);
  }

  return 1;
}

static int
decode_2nd_header (file_content *content)
{
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t x, y, something;
  uint32_t dw1, dw2;

  printf ("Decoding second header\n");

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

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  printf ("  DWORDS %i, %i\n", dw1, dw2);

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

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

  string = content_get_n_wchars (content, 10);
  if (string == NULL) return 0;
  printf ("  Font is %s\n", string);
  g_free (string);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  if (!content_get_uint32 (content, &dw3)) return 0;
  if (!content_get_uint32 (content, &dw4)) return 0;
  if (!content_get_uint32 (content, &dw5)) return 0;
  printf ("  DWORDS %i, %i, %i, %i, %i\n", dw1, dw2, dw3, dw4, dw5);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  if (!content_get_uint32 (content, &dw3)) return 0;
  if (!content_get_uint32 (content, &dw4)) return 0;
  if (!content_get_uint32 (content, &dw5)) return 0;
  printf ("  DWORDS %i, %i, %i, %i, %i\n", dw1, dw2, dw3, dw4, dw5);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  if (!content_get_uint32 (content, &dw3)) return 0;  /* Text object number or pin association? */
  if (!content_get_uint32 (content, &dw4)) return 0;
  printf ("  DWORDS %i, %i, %i, %i\n", dw1, dw2, dw3, dw4);

  if ((string = content_get_string (content)) == NULL) return 0;
  printf ("  Text is '%s'\n", string);
  g_free (string);

  return 1;
}

static int
decode_record_12 (file_content *content)
{
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t something;

  printf ("Decoding record type 12 (unknown meaning)\n");

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

  if ((string = content_get_string (content)) == NULL) return 0;
  printf ("  Pin '%s'\n", string);
  g_free (string);

  if (!content_get_byte (content, &b1)) return 0;
  printf ("  BYTE %i\n", b1);
  if (!content_get_uint32 (content, &dw1)) return 0;
  printf ("  DWORD %i\n", dw1);

  if ((string = content_get_string (content)) == NULL) return 0;
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

static void
decode_data (file_content *content)
{
  uint8_t byte;

  printf ("Decoding data stream\n");

  /* File starts with a footprint name header */
  if (!decode_name (content))
    goto error;

  while (content->cursor < content->length) {

    if (!content_get_byte (content, &byte))
      goto error;

    switch (byte) {
      case 0:
        if (!decode_attribute_record (content))
          goto error;
        break;

      case 1:
        if (!decode_2nd_header (content))
          goto error;
        break;

      case 2:
        if (!decode_pin_record (content))
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

      case 12:
        if (!decode_record_12 (content))
          goto error;
        break;

      default:
        fprintf (stderr, "Unknown section header %i at position 0x%x\n", byte,
                 content->cursor - 1);
        goto error;
    }
  }

  return;

error:
  fprintf (stderr, "Oops\n");
  return;
}

static void
print_usage (char *program)
{
  fprintf (stderr, "Usage: %s [OPTIONS] -f [datafile]\n", program);
  fprintf (stderr, "OPTIONS: -h, --help   Display usage\n");
}

int
main (int argc, char **argv)
{
  extern char *optarg;
  extern int optind, opterr, optopt;

  file_content content;
  char *optstring = "f:h";
  int opt;
  int option_index = 0;
  struct option long_options[] = {
    {"file", 1, 0, 'f'},
    {"help", 0, 0, 'h'}
  };
  char *filename = NULL;
  char *data;
  gsize length;
  GError *error = NULL;


  while ((opt = getopt_long (argc, argv, optstring,
                            long_options, &option_index)) != -1) {
    switch (opt) {
      case 'f':
        filename = g_strdup (optarg);
      break;

      case 'h':
      default: /* '?' */
        print_usage (argv[0]);
        exit (EXIT_FAILURE);
    }
  }

  if (optind < argc) {
    printf ("Non-option ARGV-elements: ");
    while (optind < argc) printf ("%s ", argv[optind++]);
    printf ("\n");
  }

  if (filename == NULL) {
    fprintf (stderr, "No filename specified\n");
    print_usage (argv[0]);
    exit (EXIT_FAILURE);
  } else {
    fprintf (stderr, "Loading from file '%s'\n", filename);
  }

  if (!g_file_get_contents (filename, &data, &length, &error)) {
    fprintf (stderr, "Error: %s\n", error->message);
    g_error_free (error);
    goto cleanup;
  }
  content.data = data;
  content.length = length;
  content.cursor = 0;
  decode_data (&content);
  g_free (data);

cleanup:
  g_free (filename);

  exit (EXIT_SUCCESS);
}
