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
#include <getopt.h>
#include <glib.h>
#include <gsf/gsf.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-msole.h>

#include "content-parser.h"
#include "pcblib-data.h"

static int
check_gerror (GError *error) {
  if (error != NULL) {
    fprintf (stderr, "Error: %s\n", error->message);
    g_error_free (error);
    return 0;
  }
  return 1;
}

static file_content *
input_to_content (GsfInput *input)
{
  file_content *content;
  size_t size;
  guint8 const *buffer;

  content = g_new0 (file_content, 1);
  content->cursor = 0;
  content->length = gsf_input_size (input);
  content->data = (char *)gsf_input_read (input, content->length, NULL);
  if (content->data == NULL) {
    fprintf (stderr, "Read error grabbing data\n");
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

static uint32_t
parse_header (GsfInput *header)
{
  file_content *content = input_to_content (header);
  uint32_t data;

  if (!content_get_uint32 (content, &data)) {
    fprintf (stderr, "Error reading size from header\n");
    exit (EXIT_FAILURE);
  }

  free_content (content);
  return data;
}

static void
parse_footprint_resource_data (GsfInput *input, int expected_sections)
{
  file_content *content = input_to_content (input);
  /* DEBUG */
  g_file_set_contents ("Data.debug", content->data, content->length, NULL);
  decode_data (content, expected_sections);
  free_content (content);
}

static void
parse_footprint_resource (GsfInfile *infile)
{
  GsfInput *header;
  GsfInput *data;

  uint32_t record_count;

  header = gsf_infile_child_by_name (infile, "Header");
  if (header == NULL) {
    fprintf (stderr, "Error: Couldn't open 'Header' file\n");
    return;
  }
  record_count = parse_header (header);
  g_object_unref (header);

  printf ("Footprint data has %i record(s)\n", record_count);

  data = gsf_infile_child_by_name (infile, "Data");
  if (data == NULL) {
    fprintf (stderr, "Error: Couldn't open 'Data' file\n");
    return;
  }
  parse_footprint_resource_data (data, record_count);
  g_object_unref (data);
}

/* Convert an Altium footprint name to the name of its resource in the file */
static char *
footprint_name_to_resource_name (const char *footprint_name)
{
  /* Translitterate '/' to '_' */
  return g_strdelimit (g_strdup (footprint_name), "/", '_');
}


static void
parse_library_resource_data (GsfInput *data)
{
  file_content *content = input_to_content (data);
  char *parameters;
  uint32_t num_footprints;
  int i;

  parameters = content_get_length_dword_prefixed_string (content);
  if (parameters == NULL) {
    fprintf (stderr, "Error getting parameters\n");
    exit (EXIT_FAILURE);
  }
  printf ("Parameters: '%s'\n", parameters);

  if (!content_get_uint32 (content, &num_footprints)) {
    fprintf (stderr, "Error getting num_footprints\n");
    exit (EXIT_FAILURE);
  }

  for (i = 0; i < num_footprints; i++) {
    char *footprint_name;
    char *resource_name;
    GsfInfile *root;
    GsfInfile *footprint;
    footprint_name = content_get_length_multi_prefixed_string (content);
    if (footprint_name == NULL) {
      fprintf (stderr, "Error getting footprint name\n");
      exit (EXIT_FAILURE);
    }
    printf ("Footprint %i: '%s'\n", i + 1, footprint_name);
    resource_name = footprint_name_to_resource_name (footprint_name);
    root = gsf_input_container (GSF_INPUT (gsf_input_container (data)));
    footprint = GSF_INFILE (gsf_infile_child_by_name (root, resource_name));
    parse_footprint_resource (footprint);
    g_object_unref (footprint);
    g_free (footprint_name);
    g_free (resource_name);
  }

  free_content (content);
}

/* Spit out the data from the 'Library' resource */
static void
parse_library_resource (GsfInfile *root)
{
  GsfInfile *infile;
  GsfInput *header;
  GsfInput *data;

  uint32_t record_count;
  char *parameters;
  uint32_t num_footprints;

  /* XXX: Should we be passed the "Library" dir directly? */
  infile = GSF_INFILE (gsf_infile_child_by_name (root, "Library"));
  if (infile == NULL) {
    fprintf (stderr, "Error: Couldn't open Library dir\n");
    return;
  }

  header = gsf_infile_child_by_name (infile, "Header");
  if (header == NULL) {
    fprintf (stderr, "Error: Couldn't open Library/Header file\n");
    return;
  }
  record_count = parse_header (header);
  g_object_unref (header);
  g_return_if_fail (record_count == 1);

  data = gsf_infile_child_by_name (infile, "Data");
  if (data == NULL) {
    fprintf (stderr, "Error: Couldn't open Library/Header file\n");
    return;
  }
  parse_library_resource_data (data);
  g_object_unref (data);
}

static void
parse_root (GsfInfile *infile)
{
  int children;
  int i;

  children = gsf_infile_num_children (infile);
  for (i = 0; i < children; i++) {
    GsfInput *input = gsf_infile_child_by_index (infile, i);
    char const *name = gsf_input_name (input);
    GsfInfile *child_infile = GSF_IS_INFILE (input) ? GSF_INFILE (input) : NULL;
    gboolean is_dir = (child_infile != NULL) &&
                      gsf_infile_num_children (child_infile) > 0;
    if (is_dir) {
      printf ("Decending into dir '%s'\n", name);
      parse_root (child_infile);
      printf ("Done with dir '%s'\n", name);
    } else {
      printf ("Child file '%s'\n", name);
    }
  }
}

static void
parse_file (char *filename)
{
  GError *error = NULL;
  GsfInput *input;
  GsfInfile *infile;

  input = gsf_input_stdio_new (filename, &error);
  if (!check_gerror (error)) return;

  /* TODO: Check magic header? */

  infile = gsf_infile_msole_new (input, &error);
  g_object_unref (input);
  if (!check_gerror (error)) return;

  parse_root (infile);
  parse_library_resource (infile);

  g_object_unref (infile);
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

  g_type_init ();

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

  parse_file (filename);

cleanup:
  g_free (filename);

  exit (EXIT_SUCCESS);
}
