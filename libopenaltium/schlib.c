/*
 *  openaltium is a set of tools for opening Altium (TM) library files
 *  Copyright (C) 2016  Peter Clifton <Peter.Clifton@clifton-electronics.co.uk>
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
#include <string.h>
#include <getopt.h>
#include <glib.h>
#include <gio/gio.h>
#include <gsf/gsf.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-msole.h>

#include "content-parser.h"
#include "parameters.h"
#include "models.h"
#include "schlib-data.h"
#include "schlib-data.h"


#if 0
/* XXX: DUPLICATE FROM pcblib-data.c */
static void
fprint_coord (FILE *file, int32_t coord)
{
//  fprintf (file, "%.2fmm", (double)coord / 1000000. * 2.54);
  fprintf (file, "%.2fmil", (double)coord / 10000.);
}
#endif


static int
check_gerror (GError *error) {
  if (error != NULL) {
    fprintf (stdout, "Error: %s\n", error->message);
    g_error_free (error);
    return 0;
  }
  return 1;
}

static file_content *
input_to_content (GsfInput *input)
{
  file_content *content;

  content = g_new0 (file_content, 1);
  content->cursor = 0;
  content->length = gsf_input_size (input);
  /* XXX: DOES THIS BUFFER NEED TO BE COPIED - MIGHT BE BAD IF WE ARE REENTRANT?? */
  content->data = (char *)gsf_input_read (input, content->length, NULL);
  if (content->data == NULL) {
    fprintf (stdout, "Read error grabbing data\n");
    g_free (content);
    return NULL;
  }
  return content;
}

static void
free_content (file_content *content)
{
  g_free (content);
}

static void
parse_symbol_resource (FILE *file, GsfInfile *root, const char *sectionkey, int part)
{
  GsfInfile *symbol;
  GsfInput *data;
  file_content *content;
  char *outfile;

  symbol = GSF_INFILE (gsf_infile_child_by_name (root, sectionkey));
  if (symbol == NULL) {
    fprintf (stdout, "Error: Couldn't open symbol resource '%s' file\n", sectionkey);
    return;
  }

  data = gsf_infile_child_by_name (symbol, "Data");
  if (data == NULL) {
    fprintf (stdout, "Error: Couldn't open 'Data' file\n");
    return;
  }

  content = input_to_content (data);

  /* DEBUG */
  outfile = g_strdup_printf ("%s.raw", sectionkey);
  g_file_set_contents (outfile, content->data, content->length, NULL);
  g_free (outfile);

  decode_schlib_data (file, content, part);
  free_content (content);
  g_object_unref (data);
  g_object_unref (symbol);
}

/* Convert an Altium symbol name to the name of its resource in the file */
static char *
libref_to_resource_name (parameter_list *sectionkeys, const char *libref)
{
  int keycount;
  int i;
  char *fieldname;
  char *value;
  char *key = NULL;

  if (sectionkeys != NULL) {

    keycount = parameter_list_get_int (sectionkeys, "KEYCOUNT");

    for (i = 0; i < keycount; i++) {
      fieldname = g_strdup_printf ("LIBREF%i", i);
      value = parameter_list_get_string (sectionkeys, fieldname);
      g_free (fieldname);

      if (strcmp (libref, value) == 0) { /* Found a match */
        fieldname = g_strdup_printf ("SECTIONKEY%i", i);
        key = parameter_list_get_string (sectionkeys, fieldname);
        g_free (fieldname);
        break;
      }
    }
  }

  if (key == NULL)
    key = strdup (libref);

  /* Translitterate '/' to '_' */
  return g_strdelimit (key, "/", '_');
}

static parameter_list *
parse_parameter_list (GsfInfile *root, const char *name)
{
  GsfInput *data;
  file_content *content;
  char *parameter_string;
  parameter_list *parameter_list;

  data = gsf_infile_child_by_name (root, name);
  if (data == NULL) {
    fprintf (stdout, "Error: Couldn't open %s file\n", name);
    return NULL;
  }

  content = input_to_content (data);

  parameter_string = content_get_length_dword_prefixed_string (content);
  if (parameter_string == NULL) {
    fprintf (stdout, "Error reading %s\n", name);
    exit (EXIT_FAILURE);
  }

  parameter_list = parameter_list_new_from_string (parameter_string);
  g_free (parameter_string);

  free_content (content);
  g_object_unref (data);

  return parameter_list;
}

static parameter_list *
parse_sectionkeys (GsfInfile *root)
{
  GsfInput *data;
  char *name = "SectionKeys";

  data = gsf_infile_child_by_name (root, name);
  if (data == NULL) {
    printf ("No SectionKeys file!\n");
    return NULL;
  }

  g_object_unref (data);

  return parse_parameter_list (root, name);
}

/* Spit out the data from the 'Library' resource */
static void
parse_fileheader (GsfInfile *root)
{
//  GsfInput *data;
//  file_content *content;
//  char *parameter_string;
  parameter_list *fileheader_parameter_list;
  parameter_list *sectionkeys_parameter_list;
  int compcount;
  int i_comp;
  int i_part;

  fileheader_parameter_list = parse_parameter_list (root, "FileHeader");

#if 0
  data = gsf_infile_child_by_name (root, "FileHeader");
  if (data == NULL) {
    fprintf (stdout, "Error: Couldn't open FileHeader file\n");
    return;
  }

  content = input_to_content (data);

  parameter_string = content_get_length_dword_prefixed_string (content);
  if (parameter_string == NULL) {
    fprintf (stdout, "Error reading FleHeader\n");
    exit (EXIT_FAILURE);
  }

  fileheader_parameter_list = parameter_list_new_from_string (parameter_string);
  g_free (parameter_string);

  free_content (content);
  g_object_unref (data);
#endif

  sectionkeys_parameter_list = parse_sectionkeys (root);

  compcount = parameter_list_get_int (fileheader_parameter_list, "COMPCOUNT");

  /* Iterate over components */
  for (i_comp = 0; i_comp < compcount; i_comp++) {

    char *fieldname;
    char *libref;
    char *description;
    char *resource_name;
    int partcount;

    fieldname = g_strdup_printf ("LIBREF%i", i_comp);
    libref = parameter_list_get_string (fileheader_parameter_list, fieldname);
    g_free (fieldname);

    fieldname = g_strdup_printf ("%%UTF8%%COMPDESCR%i", i_comp);
    description = parameter_list_get_string (fileheader_parameter_list, fieldname);
    g_free (fieldname);

    fieldname = g_strdup_printf ("PARTCOUNT%i", i_comp);
    partcount = parameter_list_get_int (fileheader_parameter_list, fieldname);
    partcount --; /* For some reasnon, partcount appears to always be +1 from the number of actual symbol parts */
    g_free (fieldname);

    resource_name = libref_to_resource_name (sectionkeys_parameter_list, libref);

    if (resource_name == NULL) {
      fprintf (stderr, "CANNOT FIND RESOURCE NAME FOR LIBREF '%s'\n", libref);
      g_free (libref);
      continue;
    }

    printf ("Symbol libref '%s', Decription '%s', Partcount %i, resource name '%s'\n",
            libref, description, partcount, resource_name);

    for (i_part = 1; i_part <= partcount; i_part++) {
      char *resource_name_no_spaces;
      char *outname;
      FILE *outfile;
      resource_name_no_spaces = g_strdelimit (g_strdup (resource_name), " ", '_');
      outname = g_strdup_printf ("%s-%i.sym", resource_name_no_spaces, i_part);
      outfile = fopen (outname, "w");
      if (outfile == NULL) {
        fprintf (stdout, "Error opening output file %s\n", outname);
        exit (EXIT_FAILURE);
      }
      g_free (outname);

      fprintf (outfile, "v 20121203 2\n");
      parse_symbol_resource (outfile, root, resource_name, i_part);
      fclose (outfile);

    }
    g_free (resource_name);
    g_free (libref);
  }

  parameter_list_free (sectionkeys_parameter_list);
  parameter_list_free (fileheader_parameter_list);


//  free_content (content);
//  g_object_unref (data);
}

void
parse_schlib_file (char *filename)
{
  GError *error = NULL;
  GsfInput *input;
  GsfInfile *root;

  input = gsf_input_stdio_new (filename, &error);
  if (!check_gerror (error)) return;

  /* TODO: Check magic header? */

  root = gsf_infile_msole_new (input, &error);
  g_object_unref (input);
  if (!check_gerror (error)) return;

  parse_fileheader (root);

  g_object_unref (root);
}
