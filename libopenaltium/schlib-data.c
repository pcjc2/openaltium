/*
 *  openaltium is a set of tools for opening Altium (TM) library files
 *  Copyright (C) 2016  Peter Clifton <pcjc2@cam.ac.uk>
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


static int
decode_binary_record (FILE *file, file_content *content, int part)
{
  uint32_t record_length;
  uint8_t type;
  uint32_t dw1; //, dw3;
  uint32_t owner_part;
  uint8_t b1, b2, b3, b4; //, b5;
  uint8_t angle;
  int16_t w1, w2, w3, w4, w5;
  uint8_t string_length;
  char *pin_notes;
  char *pin_label;
  char *pin_number;
  char *string3;
  char *string4;
  char *string5;
  int text_size = 10;

  double x, y;
  double x1, y1;
  double x2, y2;
  int color_index = 1; /* PIN COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */

  printf ("Binary record (pin?)\n");

  content_get_uint32 (content, &record_length);

  type = record_length >> 24;
  record_length &= 0x00FFFFFF;

  printf ("Binary record type %i, length %i\n", type, record_length);

  content_get_byte (content, &b1);
  printf ("  BYTE %i\n", b1);

  content_get_uint32 (content, &dw1);
  printf ("  DWORD %i\n", dw1);

  content_get_uint32 (content, &owner_part);
  printf ("  DWORD %i (owner part)\n", owner_part);

//  content_get_uint32 (content, &dw3);
//  printf ("  DWORD %i\n", dw3);

  content_get_byte (content, &b1);
  printf ("  BYTE %i\n", b1);
  content_get_byte (content, &b1);
  printf ("  BYTE %i\n", b1);
  content_get_byte (content, &b1);
  printf ("  BYTE %i\n", b1);

  content_get_byte (content, &string_length);
  pin_notes = content_get_n_chars (content, string_length);
  printf ("  STRING '%s'\n", pin_notes);

  content_get_byte (content, &b2);
  printf ("  BYTE %i\n", b2); /* ONLY SEEN 1 */

#if 1
  content_get_byte (content, &b3);
  printf ("  BYTE %i\n", b3); /* SEEN 4 and 7 */

  content_get_byte (content, &b4);
  printf ("  BYTE %i (could this be pin rotation?)\n", b4); /* SEEN 0x20 0x22 0x28 0x2A 0x30 0x31 0x32 0x33 0x38 0x3A */
#else
  content_get_int16 (content, &w1);
  printf ("  WORD %i\n", w1);
#endif

  content_get_int16 (content, &w1);
  content_get_int16 (content, &w2);
  content_get_int16 (content, &w3);
  content_get_int16 (content, &w4);
  content_get_int16 (content, &w5);
  printf ("  WORDS %i, %i, %i, %i, %i\n", w1, w2, w3, w4, w5);

  content_get_byte (content, &string_length);
  pin_label = content_get_n_chars (content, string_length);
  printf ("  STRING '%s'\n", pin_label);

  content_get_byte (content, &string_length);
  pin_number = content_get_n_chars (content, string_length);
  printf ("  STRING '%s'\n", pin_number);

  content_get_byte (content, &string_length);
  string3 = content_get_n_chars (content, string_length);
  printf ("  STRING '%s'\n", string3);

  content_get_byte (content, &string_length);
  string4 = content_get_n_chars (content, string_length);
  printf ("  STRING '%s'\n", string4);

  content_get_byte (content, &string_length);
  printf ("string_length is %i\n", string_length);
  string5 = content_get_n_chars (content, string_length);
  printf ("  STRING '%s'\n", string5);

//  content_get_byte (content, &b5);
//  printf ("  BYTE %i\n", b5);

  /* Angle looks like:
   * b4 & 0x3 == 0: right
   * b4 & 0x3 == 1: top
   * b4 & 0x3 == 2: left
   * b4 & 0x3 == 3: bottom
   */

  /* Unknown fields:
   * b4 & 0x08 - Could this be a label visibility?
   * b4 & 0x10 - Could this be a label visibility?
   * b4 & 0x20 - Could this be a label visibility?
   */

  angle = b4 & 0x03;

  switch (angle) {
    case 0: /* Pin to right */
        x1 = (w2 + w1) * 20.;
        y1 = (w3) * 20.;
        x2 = (w2) * 20.;
        y2 = (w3) * 20.;
        /* XXX: NEED TO FLIP LABELS */
      break;
    case 1: /* Pin to top */
      x1 = (w2) * 20.;
      y1 = (w3 + w1) * 20.;
      x2 = (w2) * 20.;
      y2 = (w3 - 10) * 20.;
      /* XXX: NEED TO ROTATE LABELS */
      break;
    case 2: /* Pin to left */
      x1 = (w2 - w1) * 20.;
      y1 = (w3) * 20.;
      x2 = (w2) * 20.;
      y2 = (w3) * 20.;
      break;
    case 3: /* Pin to bottom */
      x1 = (w2) * 20.;
      y1 = (w3 - w1) * 20.;
      x2 = (w2) * 20.;
      y2 = (w3) * 20.;
      /* XXX: NEED TO ROTATE LABELS */
      break;
  }

  if (owner_part >= 1 && part != owner_part) {
    printf ("Skipping binary record which does not apply to our part\n");
    return 1;
  }

  fprintf (file, "P %i %i %i %i %i %i %i # Original orientation %#x\n",
           (int)x1, (int)y1,
           (int)x2, (int)y2,
           color_index,
           0 /* NORMAL PIN */,
           0 /* WHICH END */,
           b4);

  fprintf (file, "{\n");

  x = x1 + 50;
  y = y1 + 50;
  fprintf (file, "T %i %i %i %i %i %i %i %i %i\n",
           (int)x, (int)y,
           3 /* GRAPHIC COLOR INDEX */,
           text_size,
           1 /* VISIBLE */,
           1 /* SHOW NAME ONLY */,
           0 /* ANGLE */,
           0 /* ALIGNMENT */,
           1);
  fprintf (file, "pinlabel=%s\n", pin_label);

  x = x1 - 50;
  y = y1 + 50;
  fprintf (file, "T %i %i %i %i %i %i %i %i %i\n",
           (int)x, (int)y,
           5 /* ATTRIBUTE COLOR INDEX */,
           text_size,
           1 /* VISIBLE */,
           1 /* SHOW NAME ONLY */,
           0 /* ANGLE */,
           6 /* ALIGNMENT */,
           1);
  fprintf (file, "pinnumber=%s\n", pin_number);

  fprintf (file, "}\n");

  g_free (pin_notes);
  g_free (pin_label);
  g_free (pin_number);

  return 1;
}

static int
decode_record_1 (FILE *file, parameter_list *params)
{
  char *libreference;
  char *description;

  printf ("Record 1\n");

  libreference = parameter_list_get_string (params, "LIBREFERENCE");
  fprintf (file, "#LIBREFERENCE=%s\n", libreference);
  g_free (libreference);

  description = parameter_list_get_string (params, "%UTF8%COMPONENTDESCRIPTION");
  fprintf (file, "#DESCRIPTION=%s\n", description);
  g_free (description);

  return 1;
}

static int
decode_record_3 (FILE *file, parameter_list *params)
{
  double x;
  double y;
  int symbol;
  int scalefactor;
  int linewidth;
  int color_index = 9; /* TEXT COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int size = 10; /* PLACEHOLDER - NEED TO CROSS-REF FONT SETUP IN HEADERS?? */
  bool hidden;

  printf ("Record 3 - symbol?\n"); /* XXX: Need to implement something! */

  x = parameter_list_get_double (params, "LOCATION.X") * 20. + parameter_list_get_double (params, "LOCATION.X_FRAC") * 20. / 100000.;
  y = parameter_list_get_double (params, "LOCATION.Y") * 20. + parameter_list_get_double (params, "LOCATION.Y_FRAC") * 20. / 100000.;
  symbol = parameter_list_get_int (params, "SYMBOL");
  scalefactor = parameter_list_get_int (params, "SCALEFACTOR");
  linewidth = parameter_list_get_double (params, "LINEWIDTH") * 20.;

  hidden = false;

  fprintf (file, "T %i %i %i %i %i %i %i %i %i\n",
           (int)x, (int)y,
           color_index,
           size,
           hidden ? 0 : 1,
           0 /* SHOW BOTH NAME AND VALUE */,
           0 /* ANGLE */,
           0 /* ALIGNMENT */,
           1);
  fprintf (file, "*%i*\n", symbol);

  return 1;
}

static int
decode_record_4 (FILE *file, parameter_list *params)
{
  char *text;
  double x;
  double y;
  int color_index = 9; /* TEXT COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int size = 10; /* PLACEHOLDER - NEED TO CROSS-REF FONT SETUP IN HEADERS?? */
  bool hidden;
//  int justification;
  int num_lines;

  printf ("Record 4 - label / attribute?\n");

  x = parameter_list_get_double (params, "LOCATION.X") * 20. + parameter_list_get_double (params, "LOCATION.X_FRAC") * 20. / 100000.;
  y = parameter_list_get_double (params, "LOCATION.Y") * 20. + parameter_list_get_double (params, "LOCATION.Y_FRAC") * 20. / 100000.;
  text = parameter_list_get_string (params, "%UTF8%TEXT");
  hidden = false;
//  justification = parameter_list_get_int (params, "JUSTIFICATION"); /* XXX: NEED TO MAP THIS TO GSCHEM POSITIONS */
  num_lines = 1; /* XXX: NEED TO COUNT NEWLINES IN THE STRING? */

  fprintf (file, "T %i %i %i %i %i %i %i %i %i\n",
           (int)x, (int)y,
           color_index,
           size,
           hidden ? 0 : 1,
           0 /* SHOW BOTH NAME AND VALUE */,
           0 /* ANGLE */,
           0 /* ALIGNMENT */,
           num_lines);
  fprintf (file, "%s\n", text);

  g_free (text);

  return 1;
}

static int
decode_record_5 (FILE *file, parameter_list *params)
{
  int locationcount;
  int i;
  int color_index = 2; /* GRAPHIC COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int linewidth;
  int capstyle = 2; /* XXX: ROUND */
  int dashstyle = 0; /* XXX: SOLID */
  double dashlength = 0.; /* XXX */
  double dashspace = 0.; /* XXX */


  printf ("Record 5 - bezier-curve / path?\n"); /* Bezier curve according to kicad2altium */

  locationcount = parameter_list_get_int (params, "LOCATIONCOUNT");
  linewidth = parameter_list_get_double (params, "LINEWIDTH") * 20.;
  if (linewidth <= 0) linewidth = 1;

  fprintf (file, "H %i %i %i %i %i %i %i %i %i %i %i %i %i # FROM RECORD=5\n",
           color_index,
           linewidth,
           capstyle,
           dashstyle,
           (int)dashlength,
           (int)dashspace,
           0 /* FILLING HOLLOW */,
           0 /* FILL WIDTH */,
           0 /* ANGLE 1 */,
           0 /* PITCH 1 */,
           0 /* ANGLE 2 */,
           0 /* PITCH 2 */,
           locationcount /* NUM LINES */);
//           locationcount + 1 /* NUM LINES */);

  for (i = 1; i <= locationcount; i++) {
    char *fieldname1;
    char *fieldname2;
    double x, y;

    fieldname1 = g_strdup_printf ("X%i", i);
    fieldname2 = g_strdup_printf ("X_FRAC%i", i);
    x = parameter_list_get_double (params, fieldname1) * 20. + parameter_list_get_double (params, fieldname2) * 20. / 100000.;
    g_free (fieldname1);
    g_free (fieldname2);
    fieldname1 = g_strdup_printf ("Y%i", i);
    fieldname2 = g_strdup_printf ("Y_FRAC%i", i);
    y = parameter_list_get_double (params, fieldname1) * 20. + parameter_list_get_double (params, fieldname2) * 20. / 100000.;
    g_free (fieldname1);
    g_free (fieldname2);

    fprintf (file, "%c (%i, %i)\n", (i == 1) ? 'M' : 'T', (int)x, (int)y);
  }

//  fprintf (file, "z\n");

  return 1;
}

static int
decode_record_6 (FILE *file, parameter_list *params)
{
  int color_index = 3; /* GRAPHIC COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int linewidth;
  int capstyle = 2; /* XXX: ROUND */
  int dashstyle = 0; /* XXX: SOLID */
  double dashlength = 0.; /* XXX */
  double dashspace = 0.; /* XXX */
  int locationcount;
  int i;

  printf ("Record 6 - poly line\n");

  locationcount = parameter_list_get_int (params, "LOCATIONCOUNT");
  linewidth = parameter_list_get_double (params, "LINEWIDTH") * 20.;
  if (linewidth <= 0) linewidth = 1;

  fprintf (file, "H %i %i %i %i %i %i %i %i %i %i %i %i %i\n",
           color_index,
           linewidth,
           capstyle,
           dashstyle,
           (int)dashlength,
           (int)dashspace,
           0 /* FILLING HOLLOW */,
           0 /* FILL WIDTH */,
           0 /* ANGLE 1 */,
           0 /* PITCH 1 */,
           0 /* ANGLE 2 */,
           0 /* PITCH 2 */,
           locationcount /* NUM LINES */);

  for (i = 1; i <= locationcount; i++) {
    char *fieldname1;
    char *fieldname2;
    double x, y;

    fieldname1 = g_strdup_printf ("X%i", i);
    fieldname2 = g_strdup_printf ("X_FRAC%i", i);
    x = parameter_list_get_double (params, fieldname1) * 20. + parameter_list_get_double (params, fieldname2) * 20. / 100000.;
    g_free (fieldname1);
    g_free (fieldname2);
    fieldname1 = g_strdup_printf ("Y%i", i);
    fieldname2 = g_strdup_printf ("Y_FRAC%i", i);
    y = parameter_list_get_double (params, fieldname1) * 20. + parameter_list_get_double (params, fieldname2) * 20. / 100000.;
    g_free (fieldname1);
    g_free (fieldname2);

    fprintf (file, "%c (%i, %i)\n", (i == 1) ? 'M' : 'L', (int)x, (int)y);
  }

  return 1;
}

static int
decode_record_7 (FILE *file, parameter_list *params)
{
  int locationcount;
  int i;
  int color_index = 6; /* GRAPHIC COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int linewidth;
  int capstyle = 2; /* XXX: ROUND */
  int dashstyle = 0; /* XXX: SOLID */
  double dashlength = 0.; /* XXX */
  double dashspace = 0.; /* XXX */
  bool is_solid;

  printf ("Record 7 - polygon\n");

  locationcount = parameter_list_get_int (params, "LOCATIONCOUNT");
  linewidth = parameter_list_get_double (params, "LINEWIDTH") * 20.;
  if (linewidth <= 0) linewidth = 1;
  is_solid = parameter_list_get_bool (params, "ISSOLID");

  fprintf (file, "H %i %i %i %i %i %i %i %i %i %i %i %i %i\n",
           color_index,
           linewidth,
           capstyle,
           dashstyle,
           (int)dashlength,
           (int)dashspace,
           is_solid ? 1 : 0 /* FILLING SOLID / HOLLOW */,
           0 /* FILL WIDTH */,
           0 /* ANGLE 1 */,
           0 /* PITCH 1 */,
           0 /* ANGLE 2 */,
           0 /* PITCH 2 */,
           locationcount + 1 /* NUM LINES */);

  for (i = 1; i <= locationcount; i++) {
    char *fieldname1;
    char *fieldname2;
    double x, y;

    fieldname1 = g_strdup_printf ("X%i", i);
    fieldname2 = g_strdup_printf ("X_FRAC%i", i);
    x = parameter_list_get_double (params, fieldname1) * 20. + parameter_list_get_double (params, fieldname2) * 20. / 100000.;
    g_free (fieldname1);
    g_free (fieldname2);
    fieldname1 = g_strdup_printf ("Y%i", i);
    fieldname2 = g_strdup_printf ("Y_FRAC%i", i);
    y = parameter_list_get_double (params, fieldname1) * 20. + parameter_list_get_double (params, fieldname2) * 20. / 100000.;
    g_free (fieldname1);
    g_free (fieldname2);

    fprintf (file, "%c (%i, %i)\n", (i == 1) ? 'M' : 'L', (int)x, (int)y);
  }

  fprintf (file, "z\n");


  return 1;
}

static int
decode_record_8 (FILE *file, parameter_list *params)
{
  double x, y;
  double radius;
  double secondaryradius;
  int color_index = 3; /* GRAPHIC COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int linewidth;
  int capstyle = 2; /* XXX: ROUND */
  int dashstyle = 0; /* XXX: SOLID */
  double dashlength = 0.; /* XXX */
  double dashspace = 0.; /* XXX */
  bool is_solid;

  printf ("Record 8 - ellipse\n");

  x = parameter_list_get_double (params, "LOCATION.X") * 20. + parameter_list_get_double (params, "LOCATION.X_FRAC") * 20. / 100000.;
  y = parameter_list_get_double (params, "LOCATION.Y") * 20. + parameter_list_get_double (params, "LOCATION.Y_FRAC") * 20. / 100000.;
  radius = parameter_list_get_double (params, "RADIUS") * 20. + parameter_list_get_double (params, "RADIUS_FRAC") * 20. / 100000.;
  secondaryradius = parameter_list_get_double (params, "SECONDARYRADIUS") * 20. + parameter_list_get_double (params, "SECONDARYRADIUS_FRAC") * 20. / 100000.;
  linewidth = parameter_list_get_double (params, "LINEWIDTH") * 20.;
  if (linewidth <= 0) linewidth = 1;
  is_solid = parameter_list_get_bool (params, "ISSOLID");

#if 0
  fprintf (file, "V %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i\n",
           (int)x, (int)y,
           (int)radius,
           color_index,
           linewidth,
           capstyle,
           dashstyle,
           (int)dashlength,
           (int)dashspace,
           0 /* FILLING HOLLOW */,
           0 /* FILL WIDTH */,
           0 /* ANGLE 1 */,
           0 /* PITCH 1 */,
           0 /* ANGLE 2 */,
           0 /* PITCH 2 */);
#endif

  fprintf (file, "H %i %i %i %i %i %i %i %i %i %i %i %i %i # FROM RECORD 8\n",
           color_index,
           linewidth,
           capstyle,
           dashstyle,
           (int)dashlength,
           (int)dashspace,
           is_solid ? 1 : 0 /* FILLING SOLID / HOLLOW */,
           0 /* FILL WIDTH */,
           0 /* ANGLE 1 */,
           0 /* PITCH 1 */,
           0 /* ANGLE 2 */,
           0 /* PITCH 2 */,
           2 /* NUM LINES */);

  fprintf (file, "M (%i, %i) A (%i %i 0 1 1 %i %i)\n", (int)(x + radius), (int)(y),
                                                       (int)(radius), (int)(secondaryradius),
                                                       (int)(x - radius), (int)(y));
  fprintf (file, "A (%i %i 0 1 1 %i %i) z\n", (int)(radius), (int)(secondaryradius),
                                              (int)(x + radius), (int)(y));

  return 1;
}

static int
decode_record_10 (FILE *file, parameter_list *params)
{
  double x1, y1;
  double x2, y2;
  double cornerxradius;
  double corneryradius;
  int color_index = 9; /* GRAPHIC COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int linewidth;
  int capstyle = 2; /* XXX: ROUND */
  int dashstyle = 0; /* XXX: SOLID */
  double dashlength = 0.; /* XXX */
  double dashspace = 0.; /* XXX */

  printf ("Record 10 - rounded rectangle?\n");

  x1 = parameter_list_get_double (params, "LOCATION.X") * 20. + parameter_list_get_double (params, "LOCATION.X_FRAC") * 20. / 100000.;
  y1 = parameter_list_get_double (params, "LOCATION.Y") * 20. + parameter_list_get_double (params, "LOCATION.Y_FRAC") * 20. / 100000.;
  x2 = parameter_list_get_double (params, "CORNER.X") * 20. + parameter_list_get_double (params, "CORNER.X_FRAC") * 20. / 100000.;
  y2 = parameter_list_get_double (params, "CORNER.Y") * 20. + parameter_list_get_double (params, "CORNER.Y_FRAC") * 20. / 100000.;
  cornerxradius = parameter_list_get_double (params, "CORNERXRADIUS") * 20. + parameter_list_get_double (params, "CORNERXRADIUS_FRAC") * 20. / 100000.;
  corneryradius = parameter_list_get_double (params, "CORNERYRADIUS") * 20. + parameter_list_get_double (params, "CORNERYRADIUS_FRAC") * 20. / 100000.;
  linewidth = parameter_list_get_double (params, "LINEWIDTH") * 20.; /* XXX: NOT SEEN IN THE ONE I ENOUNTERED.. IS IT FILLED? */
  if (linewidth <= 0) linewidth = 1;

  fprintf (file, "H %i %i %i %i %i %i %i %i %i %i %i %i %i\n",
           color_index,
           linewidth,
           capstyle,
           dashstyle,
           (int)dashlength,
           (int)dashspace,
           0 /* FILLING HOLLOW */,
           0 /* FILL WIDTH */,
           0 /* ANGLE 1 */,
           0 /* PITCH 1 */,
           0 /* ANGLE 2 */,
           0 /* PITCH 2 */,
           5 /* NUM LINES */);

  fprintf (file, "M (%i, %i) A (%i %i 0 0 1 %i %i)\n", (int)(x1                ), (int)(y1 + corneryradius),
                                                       (int)(     cornerxradius), (int)(     corneryradius),
                                                       (int)(x1 + cornerxradius), (int)(y1                ));
  fprintf (file, "L (%i, %i)\n", (int)x2, (int)y1);
  fprintf (file, "L (%i, %i)\n", (int)x2, (int)y2);
  fprintf (file, "L (%i, %i)\n", (int)x1, (int)y2);
  fprintf (file, "z\n");

  return 1;
}

static int
decode_record_11 (FILE *file, parameter_list *params)
{
  double x, y;
  double x1, y1;
  double x2, y2;
  double radius;
  double secondaryradius;
  int color_index = 2; /* GRAPHIC COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int linewidth;
  int capstyle = 2; /* XXX: ROUND */
  int dashstyle = 0; /* XXX: SOLID */
  double dashlength = 0.; /* XXX */
  double dashspace = 0.; /* XXX */
  double startangle;
  double endangle;
  double sweepangle;

  printf ("Record 11 - elliptical arc\n");

  x = parameter_list_get_double (params, "LOCATION.X") * 20. + parameter_list_get_double (params, "LOCATION.X_FRAC") * 20. / 100000.;
  y = parameter_list_get_double (params, "LOCATION.Y") * 20. + parameter_list_get_double (params, "LOCATION.Y_FRAC") * 20. / 100000.;
  radius = parameter_list_get_double (params, "RADIUS") * 20. + parameter_list_get_double (params, "RADIUS_FRAC") * 20. / 100000.;
  secondaryradius = parameter_list_get_double (params, "SECONDARYRADIUS") * 20. + parameter_list_get_double (params, "SECONDARYRADIUS_FRAC") * 20. / 100000.;
  startangle = parameter_list_get_double (params, "STARTANGLE");
  endangle = parameter_list_get_double (params, "ENDANGLE");
  sweepangle = endangle - startangle; /* XXX: FIXME */
  linewidth = parameter_list_get_double (params, "LINEWIDTH") * 20.;
  if (linewidth <= 0) linewidth = 1;

  fprintf (file, "H %i %i %i %i %i %i %i %i %i %i %i %i %i\n",
           color_index,
           linewidth,
           capstyle,
           dashstyle,
           (int)dashlength,
           (int)dashspace,
           0 /* FILLING HOLLOW */,
           0 /* FILL WIDTH */,
           0 /* ANGLE 1 */,
           0 /* PITCH 1 */,
           0 /* ANGLE 2 */,
           0 /* PITCH 2 */,
           1 /* NUM LINES */);

  /* XXX: IS THE ELIPSE AXIS ALIGNED? */
  x1 = x + radius * cos (startangle * M_PI / 180.);
  y1 = y + secondaryradius * sin (startangle * M_PI / 180.);
  x2 = x + radius * cos (endangle * M_PI / 180.);
  y2 = y + secondaryradius * sin (endangle * M_PI / 180.);

  fprintf (file, "M (%i, %i) A (%i %i 0 0 1 %i %i)\n", (int)(x1), (int)(y1),
                                                       (int)(radius), (int)(secondaryradius),
                                                       (int)(x2), (int)(y2));
  return 1;
}

static int
decode_record_12 (FILE *file, parameter_list *params)
{
  double x, y;
  double radius;
  int color_index = 3; /* GRAPHIC COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int linewidth;
  int capstyle = 2; /* XXX: ROUND */
  int dashstyle = 0; /* XXX: SOLID */
  double dashlength = 0.; /* XXX */
  double dashspace = 0.; /* XXX */
  double startangle;
  double endangle;
  double sweepangle;

  printf ("Record 12 - arc\n");

  x = parameter_list_get_double (params, "LOCATION.X") * 20. + parameter_list_get_double (params, "LOCATION.X_FRAC") * 20. / 100000.;
  y = parameter_list_get_double (params, "LOCATION.Y") * 20. + parameter_list_get_double (params, "LOCATION.Y_FRAC") * 20. / 100000.;
  radius = parameter_list_get_double (params, "RADIUS") * 20. + parameter_list_get_double (params, "RADIUS_FRAC") * 20. / 100000.;
  startangle = parameter_list_get_double (params, "STARTANGLE");
  endangle = parameter_list_get_double (params, "ENDANGLE");
  sweepangle = endangle - startangle; /* XXX: FIXME */
  linewidth = parameter_list_get_double (params, "LINEWIDTH") * 20.;
  if (linewidth <= 0) linewidth = 1;

  fprintf (file, "A %i %i %i %i %i %i %i %i %i %i %i\n",
           (int)x, (int)y,
           (int)radius,
           (int)startangle,
           (int)sweepangle,
           color_index,
           linewidth,
           capstyle,
           dashstyle,
           (int)dashlength,
           (int)dashspace);

  return 1;
}

static int
decode_record_13 (FILE *file, parameter_list *params)
{
  double x1, y1;
  double x2, y2;
  int color_index = 2; /* GRAPHIC COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int linewidth;
  int capstyle = 2; /* XXX: ROUND */
  int dashstyle = 0; /* XXX: SOLID */
  double dashlength = 0.; /* XXX */
  double dashspace = 0.; /* XXX */


  printf ("Record 13 - line\n");

  x1 = parameter_list_get_double (params, "LOCATION.X") * 20. + parameter_list_get_double (params, "LOCATION.X_FRAC") * 20. / 100000.;
  y1 = parameter_list_get_double (params, "LOCATION.Y") * 20. + parameter_list_get_double (params, "LOCATION.Y_FRAC") * 20. / 100000.;
  x2 = parameter_list_get_double (params, "CORNER.X") * 20. + parameter_list_get_double (params, "CORNER.X_FRAC") * 20. / 100000.;
  y2 = parameter_list_get_double (params, "CORNER.Y") * 20. + parameter_list_get_double (params, "CORNER.Y_FRAC") * 20. / 100000.;
  linewidth = parameter_list_get_double (params, "LINEWIDTH") * 20.;
  if (linewidth <= 0) linewidth = 1;

  fprintf (file, "L %i %i %i %i %i %i %i %i %i %i\n",
           (int)x1, (int)y1,
           (int)x2, (int)y2,
           color_index,
           linewidth,
           capstyle,
           dashstyle,
           (int)dashlength,
           (int)dashspace);

  return 1;
}

static int
decode_record_14 (FILE *file, parameter_list *params)
{
  double x1, y1;
  double x2, y2;
  int color_index = 3; /* GRAPHIC COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int linewidth;
  int capstyle = 2; /* XXX: ROUND */
  int dashstyle = 0; /* XXX: SOLID */
  double dashlength = 0.; /* XXX */
  double dashspace = 0.; /* XXX */
  bool is_solid;
  int color; /* XXX: NOT SUPPORTED */
  int areacolor; /* XXX: NOT SUPPORTED */
  bool transparent; /* XXX: NOT SUPPORTED */

  printf ("Record 14 - rectangle\n");

  x1 = parameter_list_get_double (params, "LOCATION.X") * 20. + parameter_list_get_double (params, "LOCATION.X_FRAC") * 20. / 100000.;
  y1 = parameter_list_get_double (params, "LOCATION.Y") * 20. + parameter_list_get_double (params, "LOCATION.Y_FRAC") * 20. / 100000.;
  x2 = parameter_list_get_double (params, "CORNER.X") * 20. + parameter_list_get_double (params, "CORNER.X_FRAC") * 20. / 100000.;
  y2 = parameter_list_get_double (params, "CORNER.Y") * 20. + parameter_list_get_double (params, "CORNER.Y_FRAC") * 20. / 100000.;
  linewidth = parameter_list_get_double (params, "LINEWIDTH") * 20.;
  if (linewidth <= 0) linewidth = 1;
  is_solid = parameter_list_get_bool (params, "ISSOLID");
  color = parameter_list_get_int (params, "COLOR");
  areacolor = parameter_list_get_int (params, "AREACOLOR");
  transparent = parameter_list_get_bool (params, "TRANSPARENT");

  fprintf (file, "B %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i\n",
           (int)x1, (int)y1,
           (int)(x2 - x1), (int)(y2 - y1),
           color_index,
           linewidth,
           capstyle,
           dashstyle,
           (int)dashlength,
           (int)dashspace,
           is_solid ? 1 : 0 /* FILLING SOLID / HOLLOW */,
           0 /* FILL WIDTH */,
           0 /* ANGLE 1 */,
           0 /* PITCH 1 */,
           0 /* ANGLE 2 */,
           0 /* PITCH 2 */);

  return 1;
}

static int
decode_record_15 (FILE *file, parameter_list *params)
{
  double x1, y1;
  double x2, y2;
  int color_index = 4; /* LOGIC BUBBLE COLOR (DEBUG!!) - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int linewidth;
  int capstyle = 2; /* XXX: ROUND */
  int dashstyle = 0; /* XXX: SOLID */
  double dashlength = 0.; /* XXX */
  double dashspace = 0.; /* XXX */


  printf ("Record 15 - sheet symbol (kicad2altium) / line?\n"); /* Kicad2altium has this as a sheet symbol */

  x1 = parameter_list_get_double (params, "LOCATION.X") * 20. + parameter_list_get_double (params, "LOCATION.X_FRAC") * 20. / 100000.;
  y1 = parameter_list_get_double (params, "LOCATION.Y") * 20. + parameter_list_get_double (params, "LOCATION.Y_FRAC") * 20. / 100000.;
  x2 = parameter_list_get_double (params, "CORNER.X") * 20. + parameter_list_get_double (params, "CORNER.X_FRAC") * 20. / 100000.;
  y2 = parameter_list_get_double (params, "CORNER.Y") * 20. + parameter_list_get_double (params, "CORNER.Y_FRAC") * 20. / 100000.;
  linewidth = parameter_list_get_double (params, "LINEWIDTH") * 20.;
  if (linewidth <= 0) linewidth = 1;

  fprintf (file, "L %i %i %i %i %i %i %i %i %i %i\n",
           (int)x1, (int)y1,
           (int)x2, (int)y2,
           color_index,
           linewidth,
           capstyle,
           dashstyle,
           (int)dashlength,
           (int)dashspace);

  return 1;
}

static int
decode_record_34 (FILE *file, parameter_list *params)
{
  char *name;
  char *text;
  double x;
  double y;
  int color_index = 9; /* TEXT COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int size = 10; /* PLACEHOLDER - NEED TO CROSS-REF FONT SETUP IN HEADERS?? */
  bool hidden;
//  int justification;
  int num_lines;

  printf ("Record 34 - designator / attribute?\n"); /* Designator according to altium2kicad */

  x = parameter_list_get_double (params, "LOCATION.X") * 20. + parameter_list_get_double (params, "LOCATION.X_FRAC") * 20. / 100000.;
  y = parameter_list_get_double (params, "LOCATION.Y") * 20. + parameter_list_get_double (params, "LOCATION.Y_FRAC") * 20. / 100000.;
  name = parameter_list_get_string (params, "NAME");
  text = parameter_list_get_string (params, "TEXT");
  hidden = parameter_list_get_bool (params, "ISHIDDEN");
//  justification = parameter_list_get_int (params, "JUSTIFICATION"); /* XXX: NEED TO MAP THIS TO GSCHEM POSITIONS */
  num_lines = 1; /* XXX: NEED TO COUNT NEWLINES IN THE STRING? */

  fprintf (file, "T %i %i %i %i %i %i %i %i %i\n",
           (int)x, (int)y,
           color_index,
           size,
           hidden ? 0 : 1,
           0 /* SHOW BOTH NAME AND VALUE */,
           0 /* ANGLE */,
           0 /* ALIGNMENT */,
           num_lines);
  fprintf (file, "%s=%s\n", name, text);

  g_free (name);
  g_free (text);

  return 1;
}

static int
decode_record_41 (FILE *file, parameter_list *params)
{
  char *name;
  char *text;
  double x;
  double y;
  int color_index = 9; /* TEXT COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int size = 10; /* PLACEHOLDER - NEED TO CROSS-REF FONT SETUP IN HEADERS?? */
  bool hidden;
//  int justification;
  int num_lines;

  printf ("Record 41 - parameter / attribute?\n"); /* Parameter according to altium2kicad */

  x = parameter_list_get_double (params, "LOCATION.X") * 20. + parameter_list_get_double (params, "LOCATION.X_FRAC") * 20. / 100000.;
  y = parameter_list_get_double (params, "LOCATION.Y") * 20. + parameter_list_get_double (params, "LOCATION.Y_FRAC") * 20. / 100000.;
  name = parameter_list_get_string (params, "NAME");
  text = parameter_list_get_string (params, "TEXT");
  hidden = parameter_list_get_bool (params, "ISHIDDEN");
//  justification = parameter_list_get_int (params, "JUSTIFICATION"); /* XXX: NEED TO MAP THIS TO GSCHEM POSITIONS */
  num_lines = 1; /* XXX: NEED TO COUNT NEWLINES IN THE STRING? */

  fprintf (file, "T %i %i %i %i %i %i %i %i %i\n",
           (int)x, (int)y,
           color_index,
           size,
           hidden ? 0 : 1,
           0 /* SHOW BOTH NAME AND VALUE */,
           0 /* ANGLE */,
           0 /* ALIGNMENT */,
           num_lines);
  fprintf (file, "%s=%s\n", name, text);

  g_free (name);
  g_free (text);

  return 1;
}

static int
decode_record_44 (FILE *file, parameter_list *params)
{
  printf ("Record 44 - unknown - blank?\n");
  return 1;
}

static int
decode_record_45 (FILE *file, parameter_list *params)
{
  char *footprint;
  double x;
  double y;
  int color_index = 9; /* TEXT COLOR - GSCHEM DOESN'T SUPPORT ARBITRARY COLOURS */
  int size = 10; /* PLACEHOLDER - NEED TO CROSS-REF FONT SETUP IN HEADERS?? */
  int num_lines;

  printf ("Record 45 - model?\n");

  x = parameter_list_get_double (params, "LOCATION.X") * 20. + parameter_list_get_double (params, "LOCATION.X_FRAC") * 20. / 100000.;
  y = parameter_list_get_double (params, "LOCATION.Y") * 20. + parameter_list_get_double (params, "LOCATION.Y_FRAC") * 20. / 100000.;
  footprint = parameter_list_get_string (params, "MODELNAME"); /* XXX: ASSUMING THIS MATCHES THE PCB MODEL!!! */

  /* XXX: CHECK MODELDATAFILEKIND0=PCBLIB */
  /* XXX: CHECK ISCURRENT=T */
  /* XXX: CHECK ODELTYPE=PCBLIB */
  /* XXX: CHECK DATAFILECOUNT=1 */

//  justification = parameter_list_get_int (params, "JUSTIFICATION"); /* XXX: NEED TO MAP THIS TO GSCHEM POSITIONS */
  num_lines = 1; /* XXX: NEED TO COUNT NEWLINES IN THE STRING? */

  fprintf (file, "T %i %i %i %i %i %i %i %i %i\n",
           (int)x, (int)y,
           color_index,
           size,
           1 /* NOT HIDDEN FOR NOW */,
           0 /* SHOW BOTH NAME AND VALUE */,
           0 /* ANGLE */,
           0 /* ALIGNMENT */,
           num_lines);
  fprintf (file, "footprint=%s\n", footprint);

  g_free (footprint);

  return 1;
}

static int
decode_record_46 (FILE *file, parameter_list *params)
{
  printf ("Record 46 - unknown - blank?\n");
  return 1;
}

static int
decode_record_47 (FILE *file, parameter_list *params)
{
  printf ("Record 47 - unknown - blank?\n");
  return 1;
}

static int
decode_record_48 (FILE *file, parameter_list *params)
{
  printf ("Record 48 - unknown - blank?\n");
  return 1;
}

void
decode_schlib_data (FILE *file, file_content *content, int part)
{
  int section_no = 0;
  int record_type;
  int owner_part;

  printf ("Decoding data stream\n");

  while (content->cursor < content->length) {

    uint32_t peek_length;
    char *parameter_string;
    parameter_list *parameter_list;

    content_get_uint32 (content, &peek_length);
    content->cursor -= 4; /* Put the cursor back to the start of the DWORD string length */

    if (peek_length & 0x01000000) /* Binary field? */ {
      peek_length &=  0x00FFFFFF;

      if (!decode_binary_record (file, content, part))
        goto error;

//      printf ("Skipping %i bytes of binary field\n", peek_length);

      section_no ++;
      continue;
    }

    parameter_string = content_get_length_dword_prefixed_string (content);
    if (parameter_string == NULL)
      goto error;

    printf ("  Index %i: string: %s\n", section_no - 1, parameter_string); /* NB: THE FIRST INDEX IS -1! */

    parameter_list = parameter_list_new_from_string (parameter_string);

    record_type = parameter_list_get_int (parameter_list, "RECORD");
    owner_part = parameter_list_get_int (parameter_list, "OWNERPARTID");

    if (owner_part >= 1 && part != owner_part) {
      printf ("Skipping record which does not apply to our part\n");
      g_free (parameter_string);
      parameter_list_free (parameter_list);
      section_no ++;
      continue;
    }

    switch (record_type) {
      case 1:
        if (!decode_record_1 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

#if 0
      case 1: /* Schematic component according to altium2kicad */
        if (!decode_record_1 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

#if 0
      case 2: /* Pin according to altium2kicad - but so far I've only encountered binary pin records */
        if (!decode_record_2 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

      case 3:
        if (!decode_record_3 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 4:
        if (!decode_record_4 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 5:
        if (!decode_record_5 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 6:
        if (!decode_record_6 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 7:
        if (!decode_record_7 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 8:
        if (!decode_record_8 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 10:
        if (!decode_record_10 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 11:
        if (!decode_record_11 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 12:
        if (!decode_record_12 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 13:
        if (!decode_record_13 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 14:
        if (!decode_record_14 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 15:
        if (!decode_record_15 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

#if 0
      case 17: /* Power object according to altium2kicad */
        if (!decode_record_17 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

#if 0
      case 22: /* Possible ERC? altium2kicad */
        if (!decode_record_43 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

#if 0
      case 25: /* Net label according to altium2kicad */
        if (!decode_record_25 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

#if 0
      case 27: /* Wire according to altium2kicad */
        if (!decode_record_27 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

#if 0
      case 28: /* Text frame according to altium2kicad */
        if (!decode_record_28 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

#if 0
      case 29: /* Junction according to altium2kicad */
        if (!decode_record_29 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

#if 0
      case 30: /* Image according to altium2kicad */
        if (!decode_record_30 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

#if 0
      case 31: /* Sheet according to altium2kicad */
        if (!decode_record_31 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

#if 0
      case 32: /* Sheet name according to altium2kicad */
        if (!decode_record_32 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

#if 0
      case 33: /* Sheet symbol according to altium2kicad */
        if (!decode_record_33 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

      case 34:
        if (!decode_record_34 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 41:
        if (!decode_record_41 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

#if 0
      case 43: /* Possible comment? altium2kicad */
        if (!decode_record_43 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;
#endif

      case 44:
        if (!decode_record_44 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 45:
        if (!decode_record_45 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 46:
        if (!decode_record_46 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 47:
        if (!decode_record_47 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      case 48:
        if (!decode_record_48 (file, parameter_list)) {
          g_free (parameter_string);
          goto error;
        }
        break;

      default:
        printf ("Unknown record type %i - content:\n%s\n", record_type, parameter_string);
        g_free (parameter_string);
        goto error;
    }

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
