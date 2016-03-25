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

#include "content-parser.h"
#include "parameters.h"
#include "models.h"


static void
fprint_coord (FILE *file, int32_t coord)
{
//  fprintf (file, "%.2fmm", (double)coord / 1000000. * 2.54);
  fprintf (file, "%.2fmil", (double)coord / 10000.);
}

static void
print_coord (int32_t coord)
{
  fprint_coord (stdout, coord);
}

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

static int
decode_arc_record (FILE *file, file_content *content)
{
  uint8_t first_byte;
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t x, y, radius;
  int32_t thickness;
  double start_angle, end_angle;
  uint32_t dw1, dw2;

  printf ("Decoding arc record\n");

  if (!content_get_byte (content, &first_byte)) return 0;
  printf ("  BYTE %i\n", first_byte);

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

  if (!content_get_uint32 (content, &thickness)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  printf ("  Next dimension pair: ");
  print_coord (thickness);
  printf (", ");
  print_coord (dw2);
  printf ("\n");

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

#ifndef NO_EXTRA_ARC_DWORD
  if (first_byte == 56) {
    if (!content_get_uint32 (content, &dw1)) return 0;
    printf ("  DWORD %i\n", dw1);
  } else {
    g_assert (first_byte == 52);
  }
#endif

  /* XXX: GOODNESS KNOWS WHAT THE ANGLE CONVENTION IS... EXAMPLES SO FAR ARE FULL CIRCLE ARCS!!! */
  fprintf (file, "\tElementArc[");
  fprint_coord (file, x);      fprintf (file, " ");
  fprint_coord (file, -y);     fprintf (file, " ");
  fprint_coord (file, radius);  fprintf (file, " ");
  fprint_coord (file, radius); fprintf (file, " ");
  fprintf (file, "%f %f ", start_angle, end_angle - start_angle);
  fprint_coord (file, thickness); fprintf (file, "]\n");

  return 1;
}

static int
decode_record_3 (FILE *file, file_content *content)
{
  uint8_t first_byte;
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t x, y, c1, c2, c3, c4, c5, c6, c7;
  uint32_t dw1, dw2;

  printf ("Decoding record of type 3 (unknown meaning)\n");

  if (!content_get_byte (content, &first_byte)) return 0;
  printf ("  BYTE %i", first_byte);

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

  //
#if 0
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

#endif
  //
  if (!content_get_int32 (content, &c1)) return 0;
  if (!content_get_int32 (content, &c2)) return 0;
  if (!content_get_int32 (content, &c3)) return 0;
  printf ("  c1: ");
  print_coord (c1); printf (" c2: ");
  print_coord (c2); printf (" c3: ");
  print_coord (c3); printf ("\n");

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

#ifndef NO_EXTRA_SECTION_3_DWORD
  if (first_byte == 241) {
    /* SKIP MORE? */
    content_skip_bytes (content, 38);
    printf ("  Skipped 38 bytes\n");
  } else if (first_byte == 209) {
    /* SKIP DIFFERENT? */
    content_skip_bytes (content, 6);
    printf ("  Skipped 6 bytes\n");
  } else {
    g_assert (first_byte == 203); /* GUESS */
  }
//  if (!content_get_uint32 (content, &dw1)) return 0;
//  printf ("  DWORD %i\n", dw1);
#endif

  return 1;
}

static int
decode_silkline (FILE *file, file_content *content)
{
  uint32_t first_dw;
  uint8_t first_byte;
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t x1, y1, x2, y2, width;
  uint32_t dw1, dw2;

  printf ("Decoding silkline\n");

#if 0
  if (!content_get_byte (content, &first_byte)) return 0;
  printf ("  BYTE %i", first_byte);

  if (!content_get_uint16 (content, &w1)) return 0;
  if (!content_get_uint16 (content, &w2)) return 0;
  if (!content_get_uint16 (content, &w3)) return 0;
  printf ("  WORDS %i, %i, %i\n", w1, w2, w3);
#endif


  if (!content_get_uint32 (content, &first_dw)) return 0; /* Some kind of size? */
  printf ("  DWORD %i\n", first_dw);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

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

#ifndef NO_EXTRA_SILKLINE_DWORD
  if (first_dw == 45) {
    if (!content_get_uint32 (content, &dw1)) return 0;
    printf ("  DWORD %i\n", dw1);
  } else {
    g_assert (first_dw == 41);
  }
#endif

  fprintf (file, "\tElementLine[");
  fprint_coord (file, x1);    fprintf (file, " ");
  fprint_coord (file, -y1);    fprintf (file, " ");
  fprint_coord (file, x2);    fprintf (file, " ");
  fprint_coord (file, -y2);    fprintf (file, " ");
  fprint_coord (file, width); fprintf (file, "]\n");

  return 1;
}

static int
decode_text_record (FILE *file, file_content *content)
{
  uint32_t first_dw;
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t x, y, height;
  double angle;
  uint32_t dw1, dw2, dw3, dw4, dw5;
  char *text;
  char *font;
  bool kludge_mode = false;

  printf ("Decoding text record\n");

#if 0
  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  if (!content_get_uint16 (content, &w1)) return 0;
  if (!content_get_uint16 (content, &w2)) return 0;
  if (!content_get_uint16 (content, &w3)) return 0;
  printf ("  WORDS %i, %i, %i\n", w1, w2, w3);
#endif

  if (!content_get_uint32 (content, &first_dw)) return 0; /* Some kind of size? */
  printf ("  DWORD %i\n", first_dw);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

  if (first_dw == 0x20) {
    kludge_mode = true;
//    content_skip_bytes (content, 29);
//    return 1;
  }

  if (kludge_mode)
    {
      printf ("  NOT SKIPPING 10 BYTES... SPECIAL TEST KLUDGE\n");
      content_skip_bytes (content, 1);
      printf ("  Skipped 1 bytes\n");
    }
  else
    {
      content_skip_bytes (content, 10);
      printf ("  Skipped 10 bytes\n");
    }

  if (!content_get_int32 (content, &x)) return 0;
  if (!content_get_int32 (content, &y)) return 0;
  if (!content_get_int32 (content, &height)) return 0;

#if POSSIBLE_NEW_FORMAT
  if (!content_get_uint16 (content, &w1)) return 0;
  if (!content_get_double (content, &angle)) return 0;
#endif

  printf ("  Text position (");
  print_coord (x); printf (", ");
  print_coord (y); printf (") Height: ");
  print_coord (height); printf ("\n");
  printf ("  Rotation angle %f\n", angle);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;

#if POSSIBLE_NEW_FORMAT
  if (!content_get_uint16 (content, &w2)) return 0;
#endif

#ifndef POSSIBLE_NEW_FORMAT
  if (kludge_mode)
    {
      printf ("NASTY KLUDGE: Dummy bytes dw3, dw4, dw5\n");
      dw3 = dw4 = dw5 = 0;
    }
  else
    {
      if (!content_get_uint32 (content, &dw3)) return 0;
      if (!content_get_uint32 (content, &dw4)) return 0;
      if (!content_get_uint32 (content, &dw5)) return 0;
    }
  printf ("  DWORDS %i, %i, %i, %i, %i\n", dw1, dw2, dw3, dw4, dw5);
#endif

  printf (" WORD %i, DOUBLE (angle %f), DWORDS %i, %i, WORD %i\n", w1, angle, dw1, dw2, w2);

  if (first_dw != 0x20)
    {
      if (!content_get_byte (content, &byte)) return 0;
      printf ("  BYTE %i\n", byte);
    }
  else
    {
      printf ("NASTY KLUDGE: NOT READING BYTE\n");
    }

  font = content_get_n_wchars (content, 32);
  if (font == NULL) return 0;
  printf ("  Font is %s\n", font);
  g_free (font);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (kludge_mode)
    {
      printf ("NASTY KLUDGE: Dummy bytes dw2, dw3\n");
      dw2 = dw3 = 0;
    }
  else
    {
      if (!content_get_uint32 (content, &dw2)) return 0;  /* Text object number or pin association? */
      if (!content_get_uint32 (content, &dw3)) return 0;
    }
  printf ("  DWORDS %i, %i, %i\n", dw1, dw2, dw3);

  if ((text = content_get_length_multi_prefixed_string (content)) == NULL) return 0;
  printf ("  Text is '%s'\n", text);

#ifndef NO_EXTRA_TEXT_4BYTES
  if (!kludge_mode)
    {
      content_skip_bytes (content, 4);
      printf ("  Skipped 4 bytes\n");
    }
#endif

#if 0 /* PCB DOESN'T SUPPORT TEXT IN ELEMENTS! */
  fprintf (file, "\tText[");
  fprint_coord (file, x);     fprintf (file, " ");
  fprint_coord (file, -y);     fprintf (file, " ");
  fprintf (file, "0 "); /* Rotation */
  fprintf (file, "%f ", height / 400.); /* scale is in percentage of the "default", which is about 40mil high */
  fprintf (file, "\"%s\" \"\"]\n", text);
#endif

  g_free (text);

  return 1;
}

static int
decode_record_6 (FILE *file, file_content *content)
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

#ifndef NO_EXTRA_SECTION_6_DWORD
  if (!content_get_uint32 (content, &dw1)) return 0;
  printf ("  DWORD %i\n", dw1);
#endif

  return 1;
}

static int
decode_polygon_record (FILE *file, file_content *content)
{
  uint8_t first_byte;
  uint8_t byte;
  uint16_t w1, w2, w3;
  int32_t something;
  char *attributes;
  uint32_t count;
  int i;

  printf ("Decoding polygon\n");

  if (!content_get_byte (content, &first_byte)) return 0;
  printf ("  BYTE %i", first_byte);

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
decode_model_record (FILE *file, file_content *content, model_map *map)
{
  uint8_t byte;
  uint32_t record_length;
  uint16_t w1, w2, w3;
  int32_t something;
  char *parameter_string;
  parameter_list *parameter_list;
  uint32_t count;
  int i;
  char *model_id;
  model_info *info;
  double ox, oy, oz;
  double ax, ay, az;
  double rx, ry, rz;
  double tx, ty, tz;

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

  parameter_string = content_get_length_dword_prefixed_string (content);
  if (parameter_string == NULL)
    return 0;
  printf ("  Model parameter string: %s\n", parameter_string);

  parameter_list = parameter_list_new_from_string (parameter_string);
  g_free (parameter_string);

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

  model_id = parameter_list_get_string (parameter_list, "MODELID");
  info = model_map_find_by_id (map, model_id);
  g_free (model_id);

  if (info == NULL) {
    printf ("XXX: DID NOT FIND MODEL ASSOCIATED WITH THIS MODELID\n");
    return 0;
  }

  /* XXX: Lookup filename from modelid */

  ox = oy = oz = 0.0;
  ax = ay = 0.0; az = 1.0;
  rx = 1.0; ry = rz = 0.0;

  ox = -info->dx / 10000.;
  oy = -info->dy / 10000.;
  oz = -info->dz / 10000.;

  printf ("Initial transform: O(%f,%f,%f) A(%f,%f,%f) R(%f,%f,%f)\n", ox, oy, oz, ax, ay, az, rx, ry, rz);

  printf ("rotx = %f\n", info->rotx);

#if 0
  tx = ox;
  ty = (oy *  cos (info->rotx * M_PI / 180.) + oz * sin (info->rotx * M_PI / 180.));
  tz = (oy * -sin (info->rotx * M_PI / 180.) + oz * cos (info->rotx * M_PI / 180.));

  ox = tx;
  oy = ty;
  oz = tz;
#endif

#if 1
  ty = (ay *  cos (info->rotx * M_PI / 180.) + az * sin (info->rotx * M_PI / 180.));
  tz = (ay * -sin (info->rotx * M_PI / 180.) + az * cos (info->rotx * M_PI / 180.));
  ay = ty;
  az = tz;

  ty = (ry *  cos (info->rotx * M_PI / 180.) + rz * sin (info->rotx * M_PI / 180.));
  tz = (ry * -sin (info->rotx * M_PI / 180.) + rz * cos (info->rotx * M_PI / 180.));
  ry = ty;
  rz = tz;
#endif

#if 1
  tz = (az *  cos (info->roty * M_PI / 180.) + ax * sin (info->roty * M_PI / 180.));
  tx = (az * -sin (info->roty * M_PI / 180.) + ax * cos (info->roty * M_PI / 180.));
  az = tz;
  ax = tx;

  tz = (rz *  cos (info->roty * M_PI / 180.) + rx * sin (info->roty * M_PI / 180.));
  tx = (rz * -sin (info->roty * M_PI / 180.) + rx * cos (info->roty * M_PI / 180.));
  rz = tz;
  rx = tx;
#endif

#if 1
  tx = (ax *  cos (info->rotz * M_PI / 180.) + ay * sin (info->rotz * M_PI / 180.));
  ty = (ax * -sin (info->rotz * M_PI / 180.) + ay * cos (info->rotz * M_PI / 180.));
  ax = tx;
  ay = ty;

  tx = (rx *  cos (info->rotz * M_PI / 180.) + ry * sin (info->rotz * M_PI / 180.));
  ty = (rx * -sin (info->rotz * M_PI / 180.) + ry * cos (info->rotz * M_PI / 180.));
  rx = tx;
  ry = ty;
#endif
  printf ("Rotated transform: O(%f,%f,%f) A(%f,%f,%f) R(%f,%f,%f)\n", ox, oy, oz, ax, ay, az, rx, ry, rz);

  ox -= parameter_list_get_double (parameter_list, "MODEL.2D.X");
  oy -= parameter_list_get_double (parameter_list, "MODEL.2D.Y");
//  oz -= parameter_list_get_double (parameter_list, "MODEL.2D.Z");

  /* XXX: 2D rotation??? */

  printf ("2D translated transform: O(%f,%f,%f) A(%f,%f,%f) R(%f,%f,%f)\n", ox, oy, oz, ax, ay, az, rx, ry, rz);

  fprintf (file, "\tAttribute(\"PCB::3d_model::type\" \"%s\")\n", "STEP-AP214"); /* XXX: ASSUMED, BUT MAY NOT BE! */
  fprintf (file, "\tAttribute(\"PCB::3d_model::filename\" \"%s/%s\")\n", g_get_current_dir(), info->filename); /* XXX: NEED TO FIX PCB SEARCH PATHS!!! */
  fprintf (file, "\tAttribute(\"PCB::3d_model::origin\" \"%f mil %f mil %f mil\")\n", ox, oy, oz);
  fprintf (file, "\tAttribute(\"PCB::3d_model::origin::X\" \"%f mil\")\n", ox);
  fprintf (file, "\tAttribute(\"PCB::3d_model::origin::Y\" \"%f mil\")\n", oy);
  fprintf (file, "\tAttribute(\"PCB::3d_model::origin::Z\" \"%f mil\")\n", oz);
  fprintf (file, "\tAttribute(\"PCB::3d_model::axis\" \"%f %f %f\")\n", ax, ay, az);
  fprintf (file, "\tAttribute(\"PCB::3d_model::axis::X\" \"%f\")\n", ax);
  fprintf (file, "\tAttribute(\"PCB::3d_model::axis::Y\" \"%f\")\n", ay);
  fprintf (file, "\tAttribute(\"PCB::3d_model::axis::Z\" \"%f\")\n", az);
  fprintf (file, "\tAttribute(\"PCB::3d_model::ref_dir\" \"%f %f %f\")\n", rx, ry, rz);
  fprintf (file, "\tAttribute(\"PCB::3d_model::ref_dir::X\" \"%f\")\n", rx);
  fprintf (file, "\tAttribute(\"PCB::3d_model::ref_dir::Y\" \"%f\")\n", ry);
  fprintf (file, "\tAttribute(\"PCB::3d_model::ref_dir::Z\" \"%f\")\n", rz);
  fprintf (file, "\tAttribute(\"PCB::rotation\" \"0 degrees\")\n");

  parameter_list_free (parameter_list);

  return 1;
}

static int
decode_record_15 (FILE *file, file_content *content)
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
decode_pin_record (FILE *file, file_content *content)
{
  uint8_t b1, b2, b3;
  uint32_t skip_length;
  uint8_t length_bytes;
  uint16_t w1, w2;
  uint16_t type_word;
  int32_t x, y, c1, c2, c3, c4, c5, c6, c7;
  double angle;
  uint8_t style1, style2, style3;
  int32_t pad, clear, mask, drill;
  uint32_t dw1, dw2, dw3, dw4, dw5;
  char *name;
  char *string;
  uint8_t to_layer = 0xFF;
  uint8_t from_layer = 0;
  uint8_t b5, b6;
  bool pin_is_round;
  bool pin_is_hole;
  bool pin_is_smd;

  printf ("Decoding pin record\n");

  if ((name = content_get_length_multi_prefixed_string (content)) == NULL) return 0;
  printf ("  Pin '%s'\n", name);

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
  if (!content_get_byte (content, &length_bytes)) return 0;  /* Some kind of length coding? */
  printf ("  BYTES %i, %i\n", b1, length_bytes);

  if (!content_get_uint16 (content, &w1)) return 0;
  if (!content_get_uint16 (content, &w2)) return 0;
  if (!content_get_uint16 (content, &type_word)) return 0;
  printf ("  WORDS %i %i %i\n", w1, w2, type_word);

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

  if (!content_get_byte (content, &style1)) return 0;
  if (!content_get_byte (content, &style2)) return 0;
  if (!content_get_byte (content, &style3)) return 0;
  printf ("  BYTES %i %i %i (Pad shape styles?)\n", style1, style2, style3);

  if (!content_get_double (content, &angle)) return 0;
  printf ("  Rotation angle %f\n", angle);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  if (!content_get_uint32 (content, &dw3)) return 0;
  printf ("  DWORDS %i, %i, %i\n", dw1, dw2, dw3);

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

  if (length_bytes == 120)
    {
      if (!content_get_byte (content, &to_layer)) return 0;
      if (!content_get_byte (content, &b2)) return 0;
      if (!content_get_byte (content, &b3)) return 0;
      if (!content_get_byte (content, &from_layer)) return 0;
      if (!content_get_byte (content, &b5)) return 0;
      if (!content_get_byte (content, &b6)) return 0;
      printf ("BYTES %i, %i, %i, %i, %i, %i\n", to_layer, b2, b3, from_layer, b5, b6);
      if (!content_get_uint32 (content, &skip_length)) return 0;


      printf ("  DWORD %i (skip length?)\n", skip_length);
      content_skip_bytes (content, skip_length); /* XXX: POSSIBLY SKIPPING OVER USEFUL DATA!! */
      printf ("  Skipped %i bytes\n", skip_length);
//      content_skip_bytes (content, 4);
//      printf ("  Skipped 4 bytes\n");
      if (skip_length != 0) {
        printf ("******************************* NOT SURE WHY THIS IS TROUBLE???? **********************\n");
//        content_skip_bytes (content, 4);
//        exit (-1);
      }
    }
  else if (length_bytes == 114)
    {
      if (!content_get_uint32 (content, &dw1)) return 0;
      printf ("  EXTRA END DWORD %i\n", dw1);
    }
  else
    {
      g_assert (length_bytes == 110); /* GUESS? */
    }

//  pin_is_round = (type_word & 8) == 0;
  pin_is_hole = (type_word & 8) == 0; /* TOTAL GUESS!! */
  pin_is_round = (style1 == 1); /* GUESS - PERHAPS 3 STYLES ARE FROM - INNER - TO (or some combinartion).. assume all same? */
  g_assert (style1 == style2);
  g_assert (style2 == style3);

  pin_is_smd   = (to_layer == from_layer); /* GUESS? */

  if (!pin_is_smd) { //(pin_is_round) {
    printf ("XXX: Assuming pin is round?\n");
    pad = c1;        /* GUESS THIS IS X DIMENSION */
    clear = c3 - c1; /* GUESS */
    mask = c5;       /* GUESS */
    drill = c7;      /* GUESS */

    if (drill >= pad) /* Encountered some files with zero size pads to represent drills */
      pin_is_hole = true;

    if (pin_is_hole && /* XXX: Expand mask size to match drilled hole - more realistic export for holes.. but don't bodge tented vias! */
        drill > mask)
      mask = drill;

    fprintf (file, "\tPin[");
    fprint_coord (file, x);     fprintf (file, " ");
    fprint_coord (file, -y);     fprintf (file, " ");
    fprint_coord (file, pad);   fprintf (file, " ");
    fprint_coord (file, clear); fprintf (file, " ");
    fprint_coord (file, mask);  fprintf (file, " ");
    fprint_coord (file, drill); fprintf (file, " ");
    fprintf (file, "\"\" \"%s\" \"%s\"]\n", name, pin_is_hole ? "hole" : (pin_is_round ? "" : "square"));
  } else {
    int32_t x1, y1, x2, y2;
    int32_t w, h;
    int32_t tx, ty;

    printf ("XXX: Assuming \"pin\" is a rectangular pad?\n");

    if (c1 > c2)
      {
        w = (c2 - c1) / 2;
        h = 0;
        pad = c2;
        clear = c4 - c2; /* XXX: Assuming clearance is uniform gap around pad in width and height ! */
        mask = c6;       /* XXX: Assuming clearance is uniform gap around pad in width and height ! */
      }
    else
      {
        w = 0;
        h = (c1 - c2) / 2;
        pad = c1;
        clear = c3 - c1; /* XXX: Assuming clearance is uniform gap around pad in width and height ! */
        mask = c5;       /* XXX: Assuming clearance is uniform gap around pad in width and height ! */
      }

    tx = w *  cos (angle * M_PI / 180.) + h * sin (angle * M_PI / 180.);
    ty = w * -sin (angle * M_PI / 180.) + h * cos (angle * M_PI / 180.);

    x1 = x + tx;
    y1 = y + ty;
    x2 = x - tx;
    y2 = y - ty;

    /* XXX: If the pad is square, PCB can't represent its rotation! */
    if (!pin_is_round)
      printf ("XXX: Assuming the pad is at zero angle!!!\n");

    fprintf (file, "\tPad[");
    fprint_coord (file, x1);     fprintf (file, " ");
    fprint_coord (file, -y1);     fprintf (file, " ");
    fprint_coord (file, x2);     fprintf (file, " ");
    fprint_coord (file, -y2);     fprintf (file, " ");
    fprint_coord (file, pad);   fprintf (file, " ");
    fprint_coord (file, clear); fprintf (file, " ");
    fprint_coord (file, mask);  fprintf (file, " ");
    fprintf (file, "\"\" \"%s\" \"%s\"]\n", name, pin_is_round ? "" : "square");
  }

  g_free (name);

  return 1;
}

void
decode_data (FILE *file, file_content *content, int expected_sections, model_map *map)
{
  uint8_t byte;
  uint32_t length;
  int section_no = 0;

  printf ("Decoding data stream\n");

  /* File starts with a footprint name header */
  if (!decode_name (file, content))
    goto error;

  while (/*section_no < expected_sections + 1 &&*/ content->cursor < content->length) { /* RE: + 1 should we just pass the correct number? */

    uint32_t begin_cursor;
    uint32_t end_cursor;

    if (!content_get_byte (content, &byte))
      goto error;

    begin_cursor = content->cursor;

    printf ("Decoding record %i/%i\n", section_no + 1, expected_sections);
    if (section_no + 1 > expected_sections)
      printf ("HMM... WHY ARE THERE EXTRA SECTIONS??\n");

    switch (byte) {

      case 1:
        if (!decode_arc_record (file, content))
          goto error;
        break;

      case 2: /* Pad object? */
        if (!decode_pin_record (file, content))
          goto error;
        break;

      case 3:
        if (!decode_record_3 (file, content))
          goto error;
        break;

      case 4:
        if (!decode_silkline (file, content))
          goto error;
        break;

      case 5:
        if (!decode_text_record (file, content))
          goto error;
        break;

      case 6:
        if (!decode_record_6 (file, content))
          goto error;
        break;

      case 11:
        if (!decode_polygon_record (file, content))
          goto error;
        break;

      case 12:
        if (!decode_model_record (file, content, map))
          goto error;
        break;

      case 15: /* FromTo object? */
        if (!decode_record_15 (file, content))
          goto error;
        break;

      //case 0: /* Track line segment? */
      case 8: /* Net object? */
      case 9: /* Component object? */
      default:
        printf ("Unknown section header %i at position 0x%x\n", byte,
                content->cursor - 1);
//        fprintf (stderr, "Unknown section header %i at position 0x%x\n", byte,
//                 content->cursor - 1);
        goto error;
    }

    end_cursor = content->cursor;

    printf ("Section read %i bytes\n", end_cursor - begin_cursor);

    section_no ++;
  }

  return;

error:
//  fprintf (stderr, "Oops\n");
  printf ("Oops\n");
  return;
}
