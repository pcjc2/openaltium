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


/* XXX: DUPLICATE FROM pcblib-data.c */
static void
fprint_coord (FILE *file, int32_t coord)
{
//  fprintf (file, "%.2fmm", (double)coord / 1000000. * 2.54);
  fprintf (file, "%.2fmil", (double)coord / 10000.);
}


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
input_decompress_to_file (GsfInput *input, const char *file)
{
  GZlibDecompressor *decomp;
  GFile *gfile;
  GFileOutputStream *file_os;
  GOutputStream *decomp_os;
  guint8 const *data;
  gsf_off_t length;
  gsize bytes_written;
  GError *error = NULL;

  decomp = g_zlib_decompressor_new (G_ZLIB_COMPRESSOR_FORMAT_ZLIB);
  gfile = g_file_new_for_path (file);
  g_file_delete (gfile, NULL, NULL);
  file_os = g_file_create (gfile, G_FILE_CREATE_NONE, NULL, &error);
  if (!check_gerror (error)) exit (-1);
  decomp_os = g_converter_output_stream_new (G_OUTPUT_STREAM (file_os), G_CONVERTER (decomp));

  length = gsf_input_size (input);
  data = gsf_input_read (input, length, NULL);
  if (data == NULL) {
    fprintf (stdout, "Read error grabbing data\n");
    return;
  }

  g_output_stream_write_all (decomp_os, data, length, &bytes_written, NULL, NULL);
  g_output_stream_close (decomp_os, NULL, NULL);
  g_output_stream_close (G_OUTPUT_STREAM (file_os), NULL, NULL);

  g_object_unref (decomp_os);
  g_object_unref (file_os);
  g_object_unref (gfile);
  g_object_unref (decomp);
}

static void
free_content (file_content *content)
{
  g_free (content);
}

static uint32_t
parse_header (GsfInfile *dir)
{
  GsfInput *header;
  file_content *content;
  uint32_t data;

  header = gsf_infile_child_by_name (dir, "Header");
  if (header == NULL) {
    fprintf (stdout, "Error: Couldn't open 'Header' file\n");
    exit (EXIT_FAILURE);
  }

  content = input_to_content (header);

  if (!content_get_uint32 (content, &data)) {
    fprintf (stdout, "Error reading size from header\n");
    exit (EXIT_FAILURE);
  }

  free_content (content);
  g_object_unref (header);
  return data;
}

static void
parse_footprint_resource (FILE *file, GsfInfile *root, const char *resource_name, model_map *map)
{
  GsfInfile *footprint;
  uint32_t record_count;
  GsfInput *data;
  file_content *content;
  char *outfile;

  footprint = GSF_INFILE (gsf_infile_child_by_name (root, resource_name));
  if (footprint == NULL) {
    fprintf (stdout, "Error: Couldn't open footprint resource '%s' file\n", resource_name);
    return;
  }

  record_count = parse_header (footprint);
  printf ("Footprint data has %i record(s)\n", record_count);

  data = gsf_infile_child_by_name (footprint, "Data");
  if (data == NULL) {
    fprintf (stdout, "Error: Couldn't open 'Data' file\n");
    return;
  }

  content = input_to_content (data);

  /* DEBUG */
  outfile = g_strdup_printf ("%s.raw", resource_name);
  g_file_set_contents (/*"Data.debug"*/outfile, content->data, content->length, NULL);
  g_free (outfile);

  decode_schlib_data (file, content, record_count, map);
  free_content (content);
  g_object_unref (data);
  g_object_unref (footprint);
}

static model_map *
parse_library_models (GsfInfile *library)
{
  model_map *map;
  GsfInfile *models;
  GsfInput *data;
  GsfInput *step;
  uint32_t record_count;
  file_content *content;
  char *step_resource_string;
  int i;

  models = GSF_INFILE (gsf_infile_child_by_name (library, "Models"));
  if (models == NULL) {
    fprintf (stdout, "Error: Couldn't open Models dir\n");
    return NULL;
  }

  record_count = parse_header (models);

  data = gsf_infile_child_by_name (models, "Data");
  if (data == NULL) {
    fprintf (stdout, "Error: Couldn't open Models/Data file\n");
    return NULL;
  }

  content = input_to_content (data);

  map = model_map_new ();

  /* Write out the STEP files */
  for (i = 0; i < record_count; i++) {
    char *parameter_string;
    parameter_list *parameter_list;
#if 0
    char *model_filename;
    char *model_id;
    double model_rotx, model_roty, model_rotz;
    double model_dx, model_dy, model_dz;
    bool model_embed;
    unsigned int model_checksum;
#endif
    model_info *info;

    /* XXX: Read each data record into a parameters list */
    parameter_string = content_get_length_dword_prefixed_string (content);
    if (parameter_string == NULL)
      return map;

    printf ("  Model %i parameter: %s\n", i, parameter_string);

    parameter_list = parameter_list_new_from_string (parameter_string);
    g_free (parameter_string);

#if 0
    model_id =       parameter_list_get_string (parameter_list, "MODELID");
    model_rotx =     parameter_list_get_double (parameter_list, "ROTX");
    model_roty =     parameter_list_get_double (parameter_list, "ROTY");
    model_rotz =     parameter_list_get_double (parameter_list, "ROTZ");
    model_dx =    parameter_list_get_dimension (parameter_list, "DX");
    model_dy =    parameter_list_get_dimension (parameter_list, "DY");
    model_dz =    parameter_list_get_dimension (parameter_list, "DZ");
    model_checksum =    parameter_list_get_int (parameter_list, "CHECKSUM");
    model_embed =      parameter_list_get_bool (parameter_list, "EMBED");
    model_filename = parameter_list_get_string (parameter_list, "NAME");
#endif

    info = model_info_new_from_parameters (parameter_list);
    parameter_list_free (parameter_list);

    model_map_insert (map, info);

    step_resource_string = g_strdup_printf ("%i", i);
    step = gsf_infile_child_by_name (GSF_INFILE (models), step_resource_string);
    if (step == NULL) {
      fprintf (stdout, "Error: Couldn't open STEP model %s\n", step_resource_string);
      g_free (step_resource_string);
      continue;
    }
    g_free (step_resource_string);
    input_decompress_to_file (step, info->filename);
    g_object_unref (step);
  }

  g_object_unref (data);

  return map;
}

/* Convert an Altium footprint name to the name of its resource in the file */
static char *
footprint_name_to_resource_name (const char *footprint_name)
{
  /* Translitterate '/' to '_' */
  return g_strdelimit (g_strdup (footprint_name), "/", '_');
}

static void
parse_library_resource_data (GsfInfile *library, model_map *map)
{
  GsfInfile *root;
  GsfInput *data;
  file_content *content;
  char *parameters;
  uint32_t num_footprints;
  int i;
  char *outname;
  FILE *outfile;
  int32_t origin_x, origin_y;

  root = gsf_input_container (GSF_INPUT (library));

  data = gsf_infile_child_by_name (library, "Data");
  if (data == NULL) {
    fprintf (stdout, "Error: Couldn't open Library/Data file\n");
    return;
  }

  content = input_to_content (data);

  parameters = content_get_length_dword_prefixed_string (content);
  if (parameters == NULL) {
    fprintf (stdout, "Error getting parameters\n");
    exit (EXIT_FAILURE);
  }
  printf ("Parameters: '%s'\n", parameters);

  if (!content_get_uint32 (content, &num_footprints)) {
    fprintf (stdout, "Error getting num_footprints\n");
    exit (EXIT_FAILURE);
  }

  for (i = 0; i < num_footprints; i++) {
    char *footprint_name;
    char *resource_name;

    footprint_name = content_get_length_multi_prefixed_string (content);
    if (footprint_name == NULL) {
      fprintf (stdout, "Error getting footprint name\n");
      exit (EXIT_FAILURE);
    }
    printf ("Footprint %i: '%s'\n", i + 1, footprint_name);
    resource_name = footprint_name_to_resource_name (footprint_name);
    origin_x = 0;
    origin_y = 0;

    outname = g_strdup_printf ("%s.fp", resource_name);
    outfile = fopen (outname, "w");
    if (outfile == NULL) {
      fprintf (stdout, "Error opening output file %s\n", outname);
      exit (EXIT_FAILURE);
    }
    g_free (outname);

    fprintf (outfile, "Element[\"\" \"\" \"\" \"\" ");
    fprint_coord (outfile, origin_x); fprintf (outfile, " ");
    fprint_coord (outfile, origin_y); fprintf (outfile, " ");
    fprintf (outfile, "0.0 0.0 0 100 \"\"]\n");
    fprintf (outfile, "(\n");
    parse_footprint_resource (outfile, root, resource_name, map);
    fprintf (outfile, ")\n");
    fclose (outfile);
    g_free (footprint_name);
    g_free (resource_name);
  }

  free_content (content);
  g_object_unref (data);
}

/* Spit out the data from the 'Library' resource */
static void
parse_library_resource (GsfInfile *root)
{
  GsfInfile *library;

  uint32_t record_count;
  model_map *map;

  library = GSF_INFILE (gsf_infile_child_by_name (root, "Library"));
  if (library == NULL) {
    fprintf (stdout, "Error: Couldn't open Library dir\n");
    return;
  }

  record_count = parse_header (library);
  g_return_if_fail (record_count == 1);

  map = parse_library_models (library);

  parse_library_resource_data (library, map);

  model_map_free (map);
}

static void
parse_root (GsfInfile *root)
{
  int children;
  int i;
//  char *name;

  children = gsf_infile_num_children (root);
  for (i = 0; i < children; i++) {
    GsfInput *input = gsf_infile_child_by_index (root, i);
//    char const *name = gsf_input_name (input);
    GsfInfile *child = GSF_IS_INFILE (input) ? GSF_INFILE (input) : NULL;
    gboolean is_dir = (child != NULL) &&
                      gsf_infile_num_children (child) > 0;
    if (is_dir) {
//      printf ("Decending into dir '%s'\n", name);
      parse_root (child);
//      printf ("Done with dir '%s'\n", name);
//    } else {
//      printf ("Child file '%s'\n", name);
    }
  }
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

  parse_root (root);
  parse_library_resource (root);

  g_object_unref (root);
}
