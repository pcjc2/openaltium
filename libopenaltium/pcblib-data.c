/*
 *  openaltium is a set of tools for opening Altium (TM) library files
 *  Copyright (C) 2011  Peter Clifton <Peter.Clifton@clifton-electronics.co.uk>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
skip_10x_ff (file_content *content)
{
  uint16_t word;
  int i;

  printf ("  SKIPPING FFFF FFFF FFFF FFFF FFFF\n");

  for (i = 0; i < 5; i++) {
    if (!content_get_uint16 (content, &word)) return 0;
    g_assert (word == 0xFFFF);
  }

  return 1;
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
  uint32_t record_length;
  uint16_t w1, w2;
  uint8_t layer;
  uint8_t byte;
  int32_t x, y, radius;
  uint32_t thickness;
  double start_angle, end_angle, delta_angle;
  uint32_t dw1, dw2;

  printf ("arc\n");

  if (!content_get_uint32 (content, &record_length)) return 0;
  printf ("  DWORD %i (record length)\n", record_length);

  if (!content_get_byte (content, &layer)) return 0;
  printf ("  BYTE %i (layer)\n", layer);

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

  if (!skip_10x_ff (content)) return 0;

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
  printf ("  Thickness: "); print_coord (thickness); printf ("\n");

  /* XXX: Assume inserting this here for larger records, gives the ordering? (small variant discovered last) */
  if (record_length >= 52) {
    if (!content_get_uint32 (content, &dw2)) return 0;
    printf ("  Unknown dimension: "); print_coord (dw2); printf ("\n");
  }

  if (!content_get_uint16 (content, &w2)) return 0;
  printf ("  WORD %i\n", w2);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  if (record_length >= 56) {
    if (!content_get_uint32 (content, &dw1)) return 0;
    printf ("  DWORD %i (layer cache / layer number?)\n", dw1);
  }

  if (record_length != 56 &&
      record_length != 52 &&
      record_length != 48)
    g_error ("Bad record length %i\n", record_length);

  delta_angle = end_angle - start_angle;
  if (delta_angle < 0.) /* XXX: What invariants do Altium arcs have? */
    delta_angle += 360;

  /* XXX: GOODNESS KNOWS WHAT THE ANGLE CONVENTION IS... EXAMPLES SO FAR ARE FULL CIRCLE ARCS!!! */
  fprintf (file, "\tElementArc[");
  fprint_coord (file, x);      fprintf (file, " ");
  fprint_coord (file, -y);     fprintf (file, " ");
  fprint_coord (file, radius);  fprintf (file, " ");
  fprint_coord (file, radius); fprintf (file, " ");
  fprintf (file, "%f %f ", 180 + start_angle, delta_angle);
  fprint_coord (file, thickness); fprintf (file, "]\n");

  return 1;
}

static int
decode_record_3 (FILE *file, file_content *content)
{
  uint32_t record_length;
  uint8_t layer;
  uint8_t b1, b2, b3;
  uint8_t b[7];
  uint16_t w1, w2;
  int32_t x, y;
  int32_t c[12];
  int32_t dim;
  uint32_t dw1;
  int i;

  printf ("type 3 (unknown meaning) - could be paste deposition / thermal via / ...?\n");

  if (!content_get_uint32 (content, &record_length)) return 0;
  printf ("  DWORD %i (record length)\n", record_length);

  if (!content_get_byte (content, &layer)) return 0;
  printf ("  BYTE %i (layer)\n", layer);

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

  if (!skip_10x_ff (content)) return 0;

  if (!content_get_int32 (content, &x)) return 0;
  if (!content_get_int32 (content, &y)) return 0;
  printf ("  Position (");
  print_coord (x); printf (", ");
  print_coord (y); printf (")\n");

  if (!content_get_int32 (content, &c[0])) return 0;
  if (!content_get_int32 (content, &c[1])) return 0;
  printf ("  c[0]: "); print_coord (c[0]);
  printf (" c[1]: "); print_coord (c[1]); printf ("\n");

  if (!content_get_byte (content, &b1)) return 0;
  if (!content_get_byte (content, &b2)) return 0;
  if (!content_get_byte (content, &b3)) return 0;
  printf ("BYTES %i, %i, %i\n", b1, b2, b3);

  if (!content_get_int32 (content, &c[2])) return 0;
  printf ("  c[2]: "); print_coord (c[2]); printf ("\n");

  if (!content_get_uint16 (content, &w2)) return 0;
  printf ("  WORD %i\n", w2);

  if (!content_get_int32 (content, &c[3])) return 0;
  if (!content_get_int32 (content, &c[4])) return 0;
  if (!content_get_int32 (content, &c[5])) return 0;
  if (!content_get_int32 (content, &c[6])) return 0;
  if (!content_get_int32 (content, &c[7])) return 0;
  if (!content_get_int32 (content, &c[8])) return 0;
  if (!content_get_int32 (content, &c[9])) return 0;

  printf ("  c[3]: "); print_coord (c[3]);
  printf (" c[4]: "); print_coord (c[4]);
  printf (" c[5]: "); print_coord (c[5]);
  printf (" c[6]: "); print_coord (c[6]);
  printf (" c[7]: "); print_coord (c[7]); printf ("\n");
  printf ("  c[8]: "); print_coord (c[8]);
  printf (" c[9]: "); print_coord (c[9]); printf ("\n");

  if (record_length >= 203) {
    if (!content_get_byte (content, &b[0])) return 0;
    printf ("  BYTE %i\n", b[0]);
  }

  if (!content_get_int32 (content, &c[10])) return 0;
  if (!content_get_int32 (content, &c[11])) return 0;


  printf (" c[10]: "); print_coord (c[10]);
  printf (" c[11]: "); print_coord (c[11]); printf ("\n");

  if (record_length >= 203) {

//    if (!content_get_byte (content, &b[0])) return 0;
//    printf ("  BYTE %i\n", b[0]);

    /* XXX: SUSPECTED PAD / ANTIPAD SIZES ON APPROX 32 LAYERS? */

    for (i = 0; i < 32; i++) {
      if (!content_get_int32 (content, &dim)) return 0;
      printf ("  n[%i]: ", i); print_coord (dim); printf ("\n");
    }
  }

  if (record_length >= 209) {
    if (!content_get_uint32 (content, &dw1)) return 0;
    printf ("  DWORD %i\n", dw1);

    if (!content_get_byte (content, &b[1])) return 0;
    if (!content_get_byte (content, &b[2])) return 0;
    printf ("  BYTES %i, %i\n", b[1], b[2]);
  }

  if (record_length >= 241) {
    content_skip_bytes (content, 32);
    printf ("  Skipped 32 bytes\n");
  }

  if (record_length != 241 &&
      record_length != 209 &&
      record_length != 203 &&
      record_length != 74)
    g_error ("Bad record length %i\n", record_length);

  /* XXX: DEBUG OUTPUT */
  if (1) {
    int32_t w, h;
    int32_t pad, clear, mask;
    int32_t c1, c2;
    int32_t x1, y1;
    int32_t x2, y2;
    int32_t tx, ty;
    double angle = 0;

    c1 = c[0];
    c2 = c[1];

    if (c1 > c2)
      {
        w = (c2 - c1) / 2;
        h = 0;
        pad = c2;
        clear = 0; // c4 - c2; /* XXX: Assuming clearance is uniform gap around pad in width and height ! */
        mask = 0; //c6;       /* XXX: Assuming clearance is uniform gap around pad in width and height ! */
      }
    else
      {
        w = 0;
        h = (c1 - c2) / 2;
        pad = c1;
        clear = 0; //c3 - c1; /* XXX: Assuming clearance is uniform gap around pad in width and height ! */
        mask = 0; //c5;       /* XXX: Assuming clearance is uniform gap around pad in width and height ! */
      }

    tx = w *  cos (angle * M_PI / 180.) + h * sin (angle * M_PI / 180.);
    ty = w * -sin (angle * M_PI / 180.) + h * cos (angle * M_PI / 180.);

    x1 = x + tx;
    y1 = y + ty;
    x2 = x - tx;
    y2 = y - ty;


    fprintf (file, "\tPad[");
    fprint_coord (file, x1);     fprintf (file, " ");
    fprint_coord (file, -y1);     fprintf (file, " ");
    fprint_coord (file, x2);     fprintf (file, " ");
    fprint_coord (file, -y2);     fprintf (file, " ");
    fprint_coord (file, pad);   fprintf (file, " ");
    fprint_coord (file, clear); fprintf (file, " ");
    fprint_coord (file, mask);  fprintf (file, " ");
    fprintf (file, "\"\" \"%s\" \"%s\"]\n", "debug", "square,onsolder");
  }

  return 1;
}

static int
decode_silkline (FILE *file, file_content *content)
{
  uint32_t record_length;
  uint8_t layer;
  uint8_t byte;
  uint16_t w1;
  uint8_t b1, b2, b3;
  int32_t x1, y1, x2, y2, width;
  uint32_t dw1;

  printf ("silkline\n");

  if (!content_get_uint32 (content, &record_length)) return 0; /* Some kind of length? */
  printf ("  DWORD %i (record length)\n", record_length);

  if (!content_get_byte (content, &layer)) return 0;
  printf ("  BYTE %i (layer)\n", layer);

  /* From: http://beta.ivc.no/wiki/index.php/Altium_Designer
   * 33: Top Overlay
   * 69: Mechanical 13 – Top Layer Component Body Information (3D models and mechanical outlines) <paired with M14>
   * 71: Mechanical 15 – Top Layer Courtyard and Assembly Information <paired with M16>
   */

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

  if (!skip_10x_ff (content)) return 0;

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

  if (!content_get_byte (content, &b1)) return 0;
  if (!content_get_byte (content, &b2)) return 0;
  if (!content_get_byte (content, &b3)) return 0;
  printf ("  BYTES %i, %i, %i\n", b1, b2, b3);

  if (record_length >= 41) {
    if (!content_get_byte (content, &byte)) return 0;
    printf ("  BYTE %i\n", byte);
    if (!content_get_uint32 (content, &dw1)) return 0;
    printf ("  DWORD %i\n", dw1);
  }

  if (record_length >= 45) {
    if (!content_get_uint32 (content, &dw1)) return 0;
    printf ("  DWORD %i (layer cache / layer number?)\n", dw1);
  }

  if (record_length != 45 &&
      record_length != 41 &&
      record_length != 36)
    g_error ("Bad record length %i\n", record_length);


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

  if (!skip_10x_ff (content)) return 0;                 /* 30 Bytes left in super-small format */

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


#if 0
    if (dw1 == 0) { /* ???? */
      /* Something else?? */

      content_skip_bytes (content, 8);
      printf ("  Skipped 8 bytes\n");

    } else if (dw1 == 200000) {
#endif
    if (record_length >= 226) {
//      content_skip_bytes (content, 17);
//      printf ("  Skipped 17 bytes\n");
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

//    } else {
//      content->cursor -= 4;
//    }
  } else {
    g_assert (record_length == 43);
  }

  printf ("Getting text from file offset %#x\n", content->cursor);

  if ((text = content_get_length_multi_prefixed_string (content)) == NULL) return 0;

  printf ("  Text is '%s'\n", text);

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
decode_rectangle_record (FILE *file, file_content *content)
{
  uint32_t record_length;
  uint8_t layer;
  uint8_t byte;
  uint16_t w1;
  int32_t x1, y1;
  int32_t x2, y2;
  uint32_t dw1, dw2, dw3, dw4;

  printf ("rectangle\n");

  if (!content_get_uint32 (content, &record_length)) return 0;
  printf ("  DWORD %i (record length)\n", record_length);

  if (!content_get_byte (content, &layer)) return 0;
  printf ("  BYTE %i (layer)\n", layer);

  /*
   * 33: Top Overlay
   * 37: Top Solder
   */


  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

  if (!skip_10x_ff (content)) return 0;

  if (!content_get_int32 (content, &x1)) return 0;
  if (!content_get_int32 (content, &y1)) return 0;
  printf ("  Coordinate ("); print_coord (x1); printf (", "); print_coord (y1); printf (")\n");

  if (!content_get_int32 (content, &x2)) return 0;
  if (!content_get_int32 (content, &y2)) return 0;
  printf ("  Coordinate ("); print_coord (x2); printf (", "); print_coord (y2); printf (")\n");


  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  printf ("  DWORDS %i, %i\n", dw1, dw2);

  /* XXX: Unknown which field is dropped in the small record variant */
  if (record_length >= 42) {
    if (!content_get_uint32 (content, &dw3)) return 0;
    printf ("  DWORD %i\n", dw3);
  }

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  if (record_length >= 46) {
    if (!content_get_uint32 (content, &dw4)) return 0;
    printf ("  DWORD %i (layer cache / layer number?)\n", dw4);
  }

  if (record_length != 46 &&
      record_length != 42 &&
      record_length != 38)
    g_error ("Bad record length %i\n", record_length);

  /* XXX: Debug output */
  if (1) {
    uint32_t width;

    width  = 50;

    /* XXX: We don't support rectangles in our footprints! */
    fprintf (file, "\tElementLine[");
    fprint_coord (file, x1);    fprintf (file, " ");
    fprint_coord (file, -y1);    fprintf (file, " ");
    fprint_coord (file, x1);    fprintf (file, " ");
    fprint_coord (file, -y2);    fprintf (file, " ");
    fprint_coord (file, width); fprintf (file, "]\n");
    fprintf (file, "\tElementLine[");
    fprint_coord (file, x2);    fprintf (file, " ");
    fprint_coord (file, -y1);    fprintf (file, " ");
    fprint_coord (file, x2);    fprintf (file, " ");
    fprint_coord (file, -y2);    fprintf (file, " ");
    fprint_coord (file, width); fprintf (file, "]\n");
    fprintf (file, "\tElementLine[");
    fprint_coord (file, x1);    fprintf (file, " ");
    fprint_coord (file, -y1);    fprintf (file, " ");
    fprint_coord (file, x2);    fprintf (file, " ");
    fprint_coord (file, -y1);    fprintf (file, " ");
    fprint_coord (file, width); fprintf (file, "]\n");
    fprintf (file, "\tElementLine[");
    fprint_coord (file, x1);    fprintf (file, " ");
    fprint_coord (file, -y2);    fprintf (file, " ");
    fprint_coord (file, x2);    fprintf (file, " ");
    fprint_coord (file, -y2);    fprintf (file, " ");
    fprint_coord (file, width); fprintf (file, "]\n");
    fprintf (file, "\tElementLine[");
    fprint_coord (file, x1);    fprintf (file, " ");
    fprint_coord (file, -y1);    fprintf (file, " ");
    fprint_coord (file, x2);    fprintf (file, " ");
    fprint_coord (file, -y2);    fprintf (file, " ");
    fprint_coord (file, width); fprintf (file, "]\n");
    fprintf (file, "\tElementLine[");
    fprint_coord (file, x1);    fprintf (file, " ");
    fprint_coord (file, -y2);    fprintf (file, " ");
    fprint_coord (file, x2);    fprintf (file, " ");
    fprint_coord (file, -y1);    fprintf (file, " ");
    fprint_coord (file, width); fprintf (file, "]\n");
  }
  return 1;
}

static int
decode_polygon_record (FILE *file, file_content *content)
{
  uint32_t record_length;
  uint32_t fields_length;
  uint8_t layer;
  uint8_t byte;
  uint16_t w1;
  uint32_t dw1;
  int32_t something;
  char *attributes;
  size_t string_length;
  uint32_t count;
  int i;

  printf ("polygon\n");

  if (!content_get_uint32 (content, &record_length)) return 0;
  printf ("  DWORD %i (record length)\n", record_length);

  if (!content_get_byte (content, &layer)) return 0;
  printf ("  BYTE %i (layer)\n", layer);

  if (!content_get_uint16 (content, &w1)) return 0;
  printf ("  WORD %i\n", w1);

  if (!skip_10x_ff (content)) return 0;

  if (!content_get_int32 (content, &something)) return 0;
  printf ("  Something: ");
  print_coord (something); printf ("\n");

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  attributes = content_get_length_dword_prefixed_string (content);
  if (attributes == NULL)
    return 0;
  string_length = strlen (attributes);

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

  fields_length = record_length - string_length - 16 * count;

  if (fields_length >= 31) {
      if (!content_get_uint32 (content, &dw1)) return 0;
      printf ("  DWORD %i\n", dw1);
  }

  if (fields_length != 31 &&
      fields_length != 27)
    g_error ("Bad fields length %i\n", fields_length);

  return 1;
}

/* NB: Angle is in degrees, and it appears this matrix is for backward rotation! */
static void
rotate_vector_backwards_degrees (double *x, double *y, double angle)
{
  double ix = *x;
  double iy = *y;
  double s = sin (angle * M_PI / 180.);
  double c = cos (angle * M_PI / 180.);

  *x = ix *  c + iy * s;
  *y = ix * -s + iy * c;
}


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
  double ox, oy, oz;
  double ax, ay, az;
  double rx, ry, rz;
  bool body_projection;

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

  if (!skip_10x_ff (content)) return 0;

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

//  if (fields_length >= 123) {
//    content_skip_bytes (content, 28);
//    printf ("  Skipped 28 bytes\n");
//  }

  if (fields_length != 31 &&
      fields_length != 27)
    g_error ("Bad fields length %i\n", fields_length);

#if 0
  if (record_length - string_length == 111) {
    /* GOODNESS KNOWS */
    if (!content_get_uint32 (content, &dw1)) return 0;
    printf ("  DWORD %i\n", dw1);
  } else if (record_length - string_length == 95) {
    if (!content_get_uint32 (content, &dw1)) return 0;
    printf ("  DWORD %i\n", dw1);
  } else {
    g_assert (record_length - string_length == 91);
  }
#endif

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

  ox = oy = oz = 0.0;
  ax = ay = 0.0; az = 1.0;
  rx = 1.0; ry = rz = 0.0;

  /* NB: Ignore these read from the model store, instead apply those from the model record where we also have X & Y offsets */
//  ox = -info->dx / 10000.; /* NB: X doesn't exist in the model store */
//  oy = -info->dy / 10000.; /* NB: Y doesn't exist in the model store */
//  oz = -info->dz / 10000.;

  printf ("Initial transform: O(%f,%f,%f) A(%f,%f,%f) R(%f,%f,%f)\n", ox, oy, oz, ax, ay, az, rx, ry, rz);

  printf ("rotx = %f\n", info->rotx);
  printf ("roty = %f\n", info->roty);
  printf ("rotz = %f\n", info->rotz);

  if (1) {
    int angle_count = ((fabs (info->rotx) > 0.01) ? 1 : 0) +
                      ((fabs (info->roty) > 0.01) ? 1 : 0) +
                      ((fabs (info->rotz) > 0.01) ? 1 : 0);

    if (angle_count > 1) {
      printf ("MULTIPLE ROTATIONS SET  X: %f Y: %f Z: %f - CHECK ME!! %s\n", info->rotx, info->roty, info->rotz, info->filename);
//      g_warning ("Multiple rotation angles set... X: %f Y: %f Z: %f erroring out for debug purposes", info->rotx, info->roty, info->rotz);
    }
  }

/* XXX: These transforms may be backwards, e.g. implicitly negating the angles */
/* XXX: These transforms may be in the wrong order (translate, x, y, z)... no
 *      examples with multiple rotated axis were found to check ordering.. it
 *      might be ZYX, for example. Checks so far suggest that this is correct.
 */
  rotate_vector_backwards_degrees (&ay, &az, info->rotx);
  rotate_vector_backwards_degrees (&ry, &rz, info->rotx);

  rotate_vector_backwards_degrees (&az, &ax, info->roty);
  rotate_vector_backwards_degrees (&rz, &rx, info->roty);


  rotate_vector_backwards_degrees (&ax, &ay, info->rotz);
  rotate_vector_backwards_degrees (&rx, &ry, info->rotz);

  printf ("Rotated transform: O(%f,%f,%f) A(%f,%f,%f) R(%f,%f,%f)\n", ox, oy, oz, ax, ay, az, rx, ry, rz);

  ox += parameter_list_get_double (parameter_list, "MODEL.2D.X");  /* Why X positive? */
  oy -= parameter_list_get_double (parameter_list, "MODEL.2D.Y");  /* Why Y negative? */
  oz -= parameter_list_get_double (parameter_list, "MODEL.3D.DZ");  /* Why Z negative? */

  az = -az; /* Why flipping az? */
  rz = -rz; /* Why flipping rz? */

  /* NB: Naming of this field is unclear, but appears to invoke a flip of the component to the
   *     opposite board side, rotating 180 degrees about the x-axis of the pre-rotated model
   */
  body_projection = parameter_list_get_bool (parameter_list, "BODYPROJECTION");
  if (body_projection) {
    oy = -oy;
    oz = -oz;
    ay = -ay;
    az = -az;
    ry = -ry;
    rz = -rz;
    /* Hack - offset by what (might be) Altium default thickness, since the Wurth library
     *        places components on the wrong side of the board at times.. then offsets through
     *        the PCB to the opposite layer. Grr.
     */
//    oz += 0.3L / 0.0254L; /* Assuming Altium default thickness of 0.3mm */

    /* Assuming Altium default thickness of:
     *   0.01016mm Soldermask (DISCOUNTED)
     *   0.03556mm copper
     *   0.32004mm core
     *   0.03556mm copper
     *   0.01016mm Soldermask (DISCOUNTED)
     */
    //oz += 0.39116L / 0.0254L;

    /* Assuming Altium default thickness of:
     *   0.01016mm Soldermask
     *   0.03556mm copper
     *   0.32004mm core
     *   0.03556mm copper
     *   0.01016mm Soldermask
     */
    oz += 0.41148L / 0.0254L;
  }

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

#if 0
static int
decode_record_15 (FILE *file, file_content *content)
{
  uint8_t byte;
  uint32_t dw1, dw2;

  printf ("type 15 (unknown meaning)\n");

  g_assert_not_reached ();

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i", byte);

  if (!content_get_uint32 (content, &dw1)) return 0;
  if (!content_get_uint32 (content, &dw2)) return 0;
  printf ("  DWORDS %i, %i\n", dw1, dw2);

  return 1;
}
#endif

static int
decode_pin_record (FILE *file, file_content *content)
{
  uint8_t b1, b2, b3, b5, b6;
  uint8_t length_bytes;
  uint16_t w1;
  uint16_t flags;
  uint16_t type_word;
  int32_t x, y, c1, c2, c3, c4, c5, c6, c7;
  double angle;
  uint8_t style1, style2, style3;
  int32_t pad, clear, mask, drill;
  uint32_t dw1, dw2, dw3, dw4, dw5;
  char *name;
  char *string;
  uint8_t byte;
  uint8_t layer;
  uint8_t to_layer = 0xFF;
  uint8_t from_layer = 0;
  bool pin_is_round;
  bool pin_is_hole;
  bool pin_is_smd;
  uint32_t last_section_length;

  printf ("pin\n");

  if ((name = content_get_length_multi_prefixed_string (content)) == NULL) return 0; /* Most use this */
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
  printf ("  WORD %i\n", w1);

  if (!content_get_byte (content, &byte)) return 0;
  printf ("  BYTE %i\n", byte);

  if (!content_get_byte (content, &layer)) return 0;
  printf ("  BYTE %i (layer)\n", layer); /* Layer 74 seems to mean MUTLILAYER */

//  if (!content_get_uint16 (content, &flags)) return 0;
//  printf ("  WORDS %i %i\n", w1, flags);

  if (!content_get_uint16 (content, &type_word)) return 0;
  printf ("  WORD %i\n", type_word);

  if (!skip_10x_ff (content)) return 0;

  if (!content_get_int32 (content, &x)) return 0;
  if (!content_get_int32 (content, &y)) return 0;
  printf ("  Pin position (");
  print_coord (x); printf (", ");
  print_coord (y); printf (")\n");

  if (!content_get_int32 (content, &c1)) return 0; /* $pos+44 in altium2kicad */
  if (!content_get_int32 (content, &c2)) return 0; // 48
  if (!content_get_int32 (content, &c3)) return 0; // 52
  if (!content_get_int32 (content, &c4)) return 0; // 56
  if (!content_get_int32 (content, &c5)) return 0; // 60
  if (!content_get_int32 (content, &c6)) return 0; // 64
  if (!content_get_int32 (content, &c7)) return 0; // 68

  printf ("  c1: "); print_coord (c1);
  printf (" c2: "); print_coord (c2);
  printf (" c3: "); print_coord (c3);
  printf (" c4: "); print_coord (c4);
  printf (" c5: "); print_coord (c5);
  printf (" c6: "); print_coord (c6);
  printf (" c7: "); print_coord (c7); printf ("\n");

  if (!content_get_byte (content, &style1)) return 0; // 72
  if (!content_get_byte (content, &style2)) return 0; // 73
  if (!content_get_byte (content, &style3)) return 0; // 74
  printf ("  BYTES %i %i %i (Pad shape styles?)\n", style1, style2, style3);

  if (!content_get_double (content, &angle)) return 0; // 75
  printf ("  Rotation angle %f\n", angle);

  if (!content_get_uint32 (content, &dw1)) return 0; // 83
  if (!content_get_uint32 (content, &dw2)) return 0; // 87
  if (!content_get_uint32 (content, &dw3)) return 0; // 91
  printf ("  DWORDS %i, %i, %i\n", dw1, dw2, dw3);

  if (!content_get_uint16 (content, &w1)) return 0; // 95
  printf ("  WORD %i\n", w1);

  if (!content_get_uint32 (content, &dw1)) return 0; // 97
  if (!content_get_uint32 (content, &dw2)) return 0; // 101
  if (!content_get_uint32 (content, &dw3)) return 0; // 105
  if (!content_get_uint32 (content, &dw4)) return 0; // 109
  if (!content_get_uint32 (content, &dw5)) return 0; // 113
  printf ("  DWORDS %i, %i, %i, %i, %i\n", dw1, dw2, dw3, dw4, dw5);
  printf ("  (as coords: ");
  print_coord (dw1); printf (", ");
  print_coord (dw2); printf (", ");
  print_coord (dw3); printf (", ");
  print_coord (dw4); printf (", ");
  print_coord (dw5); printf (")\n");

  if (!content_get_uint32 (content, &dw1)) return 0; // 117
  if (!content_get_uint32 (content, &dw2)) return 0; // 121
  if (!content_get_uint32 (content, &dw3)) return 0; // 125
  if (!content_get_uint32 (content, &dw4)) return 0; // 129    **** altium2kicad has double "HOLEROTATION" at offset $pos+129 ****
  printf ("  DWORDS %i, %i, %i, %i\n", dw1, dw2, dw3, dw4);

  g_assert (dw4 == 0);

  if (length_bytes > 106) {

    if (length_bytes == 120)
      {
        /* XXX: Unsure if this should be above the supposed layer infos... */
        if (!content_get_uint32 (content, &dw1)) return 0;
        printf ("  DWORD %i\n", dw1);

        if (!content_get_byte (content, &to_layer)) return 0;
        if (!content_get_byte (content, &b2)) return 0;
        if (!content_get_byte (content, &b3)) return 0;
        if (!content_get_byte (content, &from_layer)) return 0;
        if (!content_get_byte (content, &b5)) return 0;
        if (!content_get_byte (content, &b6)) return 0;
        printf ("BYTES %i, %i, %i, %i, %i, %i\n", to_layer, b2, b3, from_layer, b5, b6);
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

    if (!content_get_uint32 (content, &last_section_length)) return 0;
      printf ("  DWORD %i (LAST SECTION LENGTH)\n", last_section_length);

    if (last_section_length == 596 || last_section_length == 628) {
      int i;

      /* ODD PAD WITH STACK INFO? - SEEMS TO BE MULTIPLES OF 29 RECORDS */
#if 0
      1x   54 02 00 00  <-- READ AS EXTRA END DWORD

      29x  43 84 0D 00  <- SAME AS c1 dimension from normal pin record
      29x  CA 03 0C 00  <- SAME as c2 dimension from normal pin record
      29x  02           <- SAME as shape type from normal pin record, e.g. 1=Round, 2=Rectangle, (3=Octagon?? - not seen)
      2x   01
      269x 00           <- DWORD = 4 bytes, then doubles (angle) 29 x (double) = 148 byte, 
      32x  01
      32x  00
#endif

      printf ("Remaining layer pad widths\n");
      for (i = 0; i < 29; i++) {
        if (!content_get_uint32 (content, &dw1)) return 0;
        printf ("  %i: ", i); print_coord (dw1); printf ("\n");
      }
      printf ("Remaining layer pad heights\n");
      for (i = 0; i < 29; i++) {
        if (!content_get_uint32 (content, &dw1)) return 0;
        printf ("  %i: ", i); print_coord (dw1); printf ("\n");
      }
      printf ("Remaining layer pad shapes\n");
      for (i = 0; i < 29; i++) {
        if (!content_get_byte (content, &b1)) return 0;
        printf ("  %i: %i\n", i, b1);
      }

      if (!content_get_uint16 (content, &w1)) return 0;
      printf ("  WORD %i\n", w1);
      if (!content_get_uint32 (content, &dw1)) return 0;
      printf ("  DWORD %i\n", dw1);
      if (!content_get_double (content, &angle)) return 0;
      printf ("  Rotation angle %f\n", angle);

      /* XXX: IS THIS A FIXED LENGTH SKIP, OR SHOULD WE LOOK AT THE LENGTH HEADER */
      content_skip_bytes (content,  257 + 32 + 32);
      printf ("Skipped %i bytes\n", 2 + 269 + 32 + 32);
    } else if (last_section_length == 256) {
  //    g_warning ("*** NOT HANDLED PROPERLY YET ***");
  //    content_skip_bytes (content, 256);
  //    printf ("  Skipped 256 bytes\n");
      g_assert_not_reached ();
    } else if (last_section_length == 0) {
      printf ("NO MORE TO READ\n");
    } else {
      printf ("*** LAST SECTION LENGTH OF %i\n", last_section_length);
  //    g_assert_not_reached ();
    }

    if (last_section_length == 628) { /* 32 more bytes than we already read above with the 596 case */
      content_skip_bytes (content, 32);
      printf ("  Skipped 32 bytes\n");
    }

  } else {
    g_assert (length_bytes == 106);
  }

//  pin_is_round = (type_word & 8) == 0;
  pin_is_hole = (type_word & 8) == 0; /* TOTAL GUESS!! */
  pin_is_round = (style1 == 1); /* GUESS - PERHAPS 3 STYLES ARE FROM - INNER - TO (or some combinartion).. assume all same? */
  g_assert (style1 == style2);
  g_assert (style2 == style3);

  pin_is_smd = (to_layer == from_layer); /* GUESS? */
  pin_is_smd = (flags & 256) != 0;
  pin_is_smd = (layer != 74); /* GUESS? */

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
decode_pcblib_data (FILE *file, file_content *content, int expected_sections, model_map *map)
{
  uint8_t byte;
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

    printf ("Decoding record at %#x (%i/%i) - ", begin_cursor - 1, section_no + 1, expected_sections);
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
        if (!decode_rectangle_record (file, content))
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

#if 0
      case 15: /* FromTo object? */
        if (!decode_record_15 (file, content))
          goto error;
        break;
#endif

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
  exit (-1);
  return;
}
