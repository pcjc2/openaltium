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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <math.h>
#include <string.h>

#include "content-parser.h"
#include "parameters.h"
#include "models.h"


#if 0
static void
fprint_coord (FILE *file, int32_t coord)
{
//  fprintf (file, "%.2fmm", (double)coord / 1000000. * 2.54);
  fprintf (file, "%.2fmil", (double)coord / 10000.);
}
#endif

#if 0
static void
print_coord (int32_t coord)
{
  fprint_coord (stdout, coord);
}
#endif

#if 0
static int
decode_name (FILE *file, file_content *content)
{
  char *string;

  printf ("Decoding name header\n");

  if ((string = content_get_length_multi_prefixed_string (content)) == NULL) return 0;
  printf ("  String is '%s'\n", string);

  g_free (string);
  return 1;
}
#endif

#if 0
static int
decode_text_record (FILE *file, file_content *content)
{
  uint32_t record_length;
  uint8_t layer;
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t x, y, height;
  double angle;
  uint32_t dw1, dw2, dw3, dw4, dw5, dw6, dw7;
  char *text;
  char *font = NULL;

  printf ("text\n");

  if (!content_get_uint32 (content, &record_length)) return 0; /* NB: Excludes string */
  printf ("  DWORD %i\n", record_length);

  if (!content_get_byte (content, &layer)) return 0;
  printf ("  BYTE %i (layer)\n", layer);

  if (!content_get_uint16 (content, &w3)) return 0;
  printf ("  WORD %i\n", w3);

  if (!content_get_int32 (content, &x)) return 0;      /* 26 Bytes left in super-small format */
  if (!content_get_int32 (content, &y)) return 0;      /* 22 Bytes left in super-small format */
  if (!content_get_int32 (content, &height)) return 0; /* 18 Bytes left in super-small format */
  if (!content_get_uint16 (content, &w1)) return 0;    /* 16 Bytes left in super-small format */
  if (!content_get_double (content, &angle)) return 0; /*  8 Bytes left in super-small format */

  printf ("  Text position (");
  print_coord (x); printf (", ");
  print_coord (y); printf (") Height: ");
  print_coord (height); printf ("\n");
  printf ("  WORD %i\n", w1);
  printf ("  Rotation angle %f\n", angle);

  if (!content_get_uint32 (content, &dw1)) return 0;  /* 4 Bytes left in super-small format */
  if (!content_get_uint32 (content, &dw2)) return 0;  /* 0 Bytes left in super-small format */
  printf (" DWORDS %i, %i\n", dw1, dw2);

  if (record_length >= 123) {

    if (!content_get_uint16 (content, &w2)) return 0;
    printf (" WORD %i\n", w2);

    if (!content_get_byte (content, &byte)) return 0;
    printf ("  BYTE %i\n", byte);

    font = content_get_n_wchars (content, 32);
    if (font == NULL) return 0;
    printf ("  Font is %s\n", font);
    g_free (font);

    if (!content_get_byte (content, &byte)) return 0;
    printf ("  BYTE %i\n", byte);

    if (!content_get_uint32 (content, &dw1)) return 0;
    printf ("  DWORD %i\n", dw1);

    if (!content_get_uint32 (content, &dw1)) return 0;
    printf ("  DWORD %i\n", dw1);
    if (!content_get_uint32 (content, &dw1)) return 0;
    printf ("  DWORD %i\n", dw1);


    if (record_length >= 226) {
      content_skip_bytes (content, 9);
      printf ("  Skipped 9 bytes\n");

      if (!content_get_byte (content, &byte)) return 0;
      printf ("  BYTE %i\n", byte);

      if (!content_get_uint32 (content, &dw1)) return 0;
      if (!content_get_uint32 (content, &dw2)) return 0;
      if (!content_get_uint32 (content, &dw3)) return 0;
      if (!content_get_uint32 (content, &dw4)) return 0;
      if (!content_get_uint32 (content, &dw5)) return 0;
      if (!content_get_uint32 (content, &dw6)) return 0;
      if (!content_get_uint32 (content, &dw7)) return 0;
      printf ("  DWORD %i, %i, %i, %i, %i, %i, %i\n", dw1, dw2, dw3, dw4, dw5, dw6, dw7);

      font = content_get_n_wchars (content, 32);
      if (font == NULL) return 0;
      printf ("  Font is %s\n", font);
      g_free (font);

      if (!content_get_byte (content, &byte)) return 0;
      printf ("  BYTE %i\n", byte);
    }

    if (record_length >= 230) {
      if (!content_get_uint32 (content, &dw1)) return 0;
      printf ("  DWORD %i\n", dw1);
    }

    if (record_length != 230 &&
        record_length != 226 &&
        record_length != 123)
      g_error ("Bad record length %i\n", record_length);

  } else {
    g_assert (record_length == 43);
  }

  printf ("Getting text from file offset %#x\n", content->cursor);

  if ((text = content_get_length_multi_prefixed_string (content)) == NULL) return 0;

  printf ("  Text is '%s'\n", text);

  g_free (text);

  return 1;
}
#endif

#if 0
static int
decode_model_record (FILE *file, file_content *content, model_map *map)
{
  uint32_t record_length;
  uint8_t layer;
  uint32_t fields_length;
  uint16_t w1;
  uint32_t dw1;
  int32_t something;
  uint8_t byte;
  char *parameter_string;
  size_t string_length;
  parameter_list *parameter_list;
  uint32_t count;
  int i;
  char *model_id;
  model_info *info;

  printf ("model\n");

  if (!content_get_uint32 (content, &record_length)) return 0;
  printf ("  Record length is %i\n", record_length);

  if (!content_get_byte (content, &layer)) return 0;
  printf ("  BYTE %i (model layer)\n", layer);

  /* 57: Mechanical 1  -  Board Outline (along with the Keep-Out Layer, but that can be used for other things also)
   * 69: Mechanical 13 -  Top Layer Component Body Information (3D models and mechanical outlines) <paired with M14>
   */

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

  if (!content_get_int32 (content, &something)) return 0;
  printf ("  Something: ");
  print_coord (something); printf ("\n");

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  parameter_string = content_get_length_dword_prefixed_string (content);
  if (parameter_string == NULL)
    return 0;
  printf ("  Model parameter string: %s\n", parameter_string);
  string_length = strlen (parameter_string);

  parameter_list = parameter_list_new_from_string (parameter_string);
  g_free (parameter_string);

  if (!content_get_uint32 (content, &count)) return 0;

  printf ("  Model outline (%i vertices): ", count);

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

  fields_length = record_length - string_length - 16 * count;

  if (fields_length >= 31) {
    if (!content_get_uint32 (content, &dw1)) return 0;
    printf ("  DWORD %i\n", dw1);
  }

  if (fields_length != 31 &&
      fields_length != 27)
    g_error ("Bad fields length %i\n", fields_length);

  if (!parameter_list_get_bool (parameter_list, "MODEL.EMBED")) {
    parameter_list_free (parameter_list);
    return 1;
  }

  model_id = parameter_list_get_string (parameter_list, "MODELID");
  info = model_map_find_by_id (map, model_id);
  g_free (model_id);

  if (info == NULL) {
    printf ("XXX: DID NOT FIND MODEL ASSOCIATED WITH THIS MODELID\n");
    parameter_list_free (parameter_list);
    return 0;
  }

  /* XXX: Lookup filename from modelid */

  //ox -= parameter_list_get_double (parameter_list, "MODEL.2D.X");
  //oy -= parameter_list_get_double (parameter_list, "MODEL.2D.Y");


  parameter_list_free (parameter_list);

  return 1;
}
#endif

void
decode_schlib_data (FILE *file, file_content *content, int expected_sections, model_map *map)
{
  int section_no = 0;

  printf ("Decoding data stream\n");

  while (content->cursor < content->length) {

    char *parameter_string;
    parameter_list *parameter_list;

    parameter_string = content_get_length_dword_prefixed_string (content);
    if (parameter_string == NULL)
      goto error;

    printf ("  Record %i: string: %s\n", section_no, parameter_string);

    parameter_list = parameter_list_new_from_string (parameter_string);
    g_free (parameter_string);

    parameter_list_free (parameter_list);

    section_no ++;
  }

  return;

error:
  printf ("Oops\n");
  exit (-1);
  return;
}
