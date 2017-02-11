/*
 *  openaltium is a set of tools for opening Altium (TM) library files
 *  Copyright (C) 2011-2016  Peter Clifton <Peter.Clifton@clifton-electronics.co.uk>
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
#include "pcblib.h"
#include "schlib.h"


static void
print_usage (char *program)
{
  fprintf (stdout, "Usage: %s [OPTIONS] -f [datafile]\n", program);
  fprintf (stdout, "OPTIONS: -p, --pcblib PCBLib file\n");
  fprintf (stdout, "         -s, --schlib SchLib file\n");
  fprintf (stdout, "         -h, --help   Display usage\n");
}

enum mode_e {
  MODE_NONE,
  MODE_PCBLIB,
  MODE_SCHLIB
};

int
main (int argc, char **argv)
{
  extern char *optarg;
  extern int optind, opterr, optopt;
  enum mode_e mode = MODE_NONE;

  char *optstring = "f:psh";
  int opt;
  int option_index = 0;
  struct option long_options[] = {
    {"file",   required_argument, NULL, 'f'},
    {"pcblib", no_argument,       NULL, 'p'},
    {"schlib", no_argument,       NULL, 's'},
    {"help",   no_argument,       NULL, 'h'},
    {NULL,     0,                 NULL, 0}
  };
  char *filename = NULL;

  while ((opt = getopt_long (argc, argv, optstring,
                            long_options, &option_index)) != -1) {
    switch (opt) {
      case 'f':
        filename = g_strdup (optarg);
      break;

      case 'p':
        if (mode != MODE_NONE) {
          fprintf (stdout, "More than one file mode specified\n");
          print_usage (argv[0]);
          exit (EXIT_FAILURE);
        }
        mode = MODE_PCBLIB;
      break;

      case 's':
        if (mode != MODE_NONE) {
          fprintf (stdout, "More than one file mode specified\n");
          print_usage (argv[0]);
          exit (EXIT_FAILURE);
        }
        mode = MODE_SCHLIB;
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

  if (mode == MODE_NONE) {
    fprintf (stdout, "No file type specified\n");
    print_usage (argv[0]);
    exit (EXIT_FAILURE);
  }

  if (filename == NULL) {
    fprintf (stdout, "No filename specified\n");
    print_usage (argv[0]);
    exit (EXIT_FAILURE);
  } else {
    fprintf (stdout, "Loading from file '%s'\n", filename);
  }

  switch (mode) {

    case MODE_NONE:
      g_assert_not_reached ();
      break;

    case MODE_PCBLIB:
      parse_pcblib_file (filename);
      break;

    case MODE_SCHLIB:
      parse_schlib_file (filename);
      break;

  }

  g_free (filename);

  exit (EXIT_SUCCESS);
}
