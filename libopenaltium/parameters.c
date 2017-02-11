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
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "parameters.h"

struct parameter_list {
  GVariantDict dict;
};


parameter_list *
parameter_list_new_from_string (const char *string)
{
  parameter_list *list;
  char **parameters;
  char **parameter;

  list = g_slice_new0 (parameter_list);
  g_variant_dict_init (&list->dict, NULL);

  parameters = g_strsplit ((string[0] == '|') ? &string[1] : string, "|", 0);

  for (parameter = parameters; *parameter != NULL; parameter++) {
    char **nv;
    nv = g_strsplit (*parameter, "=", 2);
    if (nv[0] != NULL &&
        nv[1] != NULL)
      if (g_utf8_validate (nv[1], -1, NULL)) {
//        printf ("%s=%s\n", nv[0], nv[1]);
        g_variant_dict_insert (&list->dict, nv[0], "s", nv[1]);
      } else {
//        printf ("Non UTF8 encoding found in parameter %s\n", nv[0]);
        g_variant_dict_insert (&list->dict, nv[0], "s", "BAD ENCODING");
        /* XXX: Should we include this as a byte array in the dictionary? */
      }
//    printf ("LISTING PARAMETER %s (Name='%s', Value='%s')\n", *parameter, nv[0], nv[1]);
    g_strfreev (nv);
  }

  g_strfreev (parameters);

  return list;
}

void
parameter_list_free (parameter_list *list)
{
  /* NB:We don't unref the GVariantDict list->dict, as it is in-place */

  g_slice_free (parameter_list, list);
}

int32_t
parameter_list_get_dimension (const parameter_list *list, const char *name)
{
  char *string = NULL;
  int32_t value = 0; /* Default return value for not found */

  if (!g_variant_dict_lookup ((/* not const */GVariantDict *)&list->dict, name, "s", &string))
    return value;

  exit (-1);
  /* XXX: Need to parse the string */
  value = -1;

  return value;
}

double
parameter_list_get_double (const parameter_list *list, const char *name)
{
  char *string = NULL;
  double value = 0.0; /* Default return value for not found */

  if (!g_variant_dict_lookup ((/* not const */GVariantDict *)&list->dict, name, "s", &string))
    return value;

  value = atof (string);

  return value;
}

unsigned int
parameter_list_get_unsigned_int (const parameter_list *list, const char *name)
{
  char *string = NULL;
  unsigned int value = 0; /* Default return value for not found */

  if (!g_variant_dict_lookup ((/* not const */GVariantDict *)&list->dict, name, "s", &string))
    return value;

  value = strtoul (string, NULL, 0); /* XXX: NO OVERFLOW HANDLING ETC */

  return value;
}

int
parameter_list_get_int (const parameter_list *list, const char *name)
{
  char *string = NULL;
  int value = 0; /* Default return value for not found */

  if (!g_variant_dict_lookup ((/* not const */GVariantDict *)&list->dict, name, "s", &string))
    return value;

  value = atoi (string);

  return value;
}

bool
parameter_list_get_bool (const parameter_list *list, const char *name)
{
  char *string = NULL;
  bool value = false; /* Default return value for not found */

  if (!g_variant_dict_lookup ((/* not const */GVariantDict *)&list->dict, name, "s", &string))
    return value;

//  if (strcmp (string, "TRUE") == 0)
//    value = true;

  if (string[0] == 'T' || string[0] == '1')
    value = true;

  return value;
}

char *
parameter_list_get_string (const parameter_list *list, const char *name)
{
  char *string;

  if (!g_variant_dict_lookup ((/* not const */GVariantDict *)&list->dict, name, "s", &string))
    return g_strdup ("");

  return g_strdup (string);
}
