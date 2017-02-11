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
decode_data (file_content *content)
{
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
