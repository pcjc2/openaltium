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
#include <glib.h>
#include <math.h>

#include "parameters.h"
#include "models.h"


struct model_map {
  GHashTable *hash;
};


model_info *
model_info_new_from_parameters (const parameter_list *list)
{
  model_info *info;
  char **path_parts;
  int i;

  info = g_slice_new0 (model_info);

  info->id =       parameter_list_get_string (list, "ID");
  info->rotx =     parameter_list_get_double (list, "ROTX");
  info->roty =     parameter_list_get_double (list, "ROTY");
  info->rotz =     parameter_list_get_double (list, "ROTZ");
//  info->dx =    parameter_list_get_dimension (list, "DX");
//  info->dy =    parameter_list_get_dimension (list, "DY");
//  info->dz =    parameter_list_get_dimension (list, "DZ");
  info->dx =    parameter_list_get_int (list, "DX"); /* XXX: FIX dimension routine */
  info->dy =    parameter_list_get_int (list, "DY"); /* XXX: FIX dimension routine */
  info->dz =    parameter_list_get_int (list, "DZ"); /* XXX: FIX dimension routine */
  info->checksum =    parameter_list_get_int (list, "CHECKSUM");
  info->embed =      parameter_list_get_bool (list, "EMBED");
  info->filename = parameter_list_get_string (list, "NAME");

  /* To assist debugigng with multiple rotation angles - help to pick them out! */
  if (fabs (info->rotx - 360.) < 0.01) info->rotx = 0.;
  if (fabs (info->roty - 360.) < 0.01) info->roty = 0.;
  if (fabs (info->rotz - 360.) < 0.01) info->rotz = 0.;

  /* Rationalise filenames which are absolute (windows) paths! */
  path_parts = g_strsplit (info->filename, "\\", 0);
  g_free (info->filename);

  for (i = 0; path_parts[i + 1] != NULL; i++); /* i will be the last index when this completes */
  info->filename = g_strdup (path_parts[i]);

  g_strfreev (path_parts);

  return info;
}

void
model_info_free (model_info *info)
{
  g_free (info->id);
  g_free (info->filename);
  g_slice_free (model_info, info);
}

model_map *
model_map_new ()
{
  model_map *map;

  map = g_slice_new0 (model_map);
  map->hash = g_hash_table_new_full ((GHashFunc) g_str_hash,
                                     (GEqualFunc) g_str_equal,
                                     NULL, /* NB: DONT FREE THE KEY, SINCE IT IS PART OF THE VALUE */
                                     NULL); // XXX: STILL NOT GOOD ENOUGH, IT USE-AFTER-FREE's THE KEY (GDestroyNotify) model_info_free);

  return map;
}

void
model_map_free (model_map *map)
{
  if (map == NULL)
    return;

  g_hash_table_unref (map->hash);
  g_slice_free (model_map, map);
}

void
model_map_insert (model_map *map, model_info *info)
{
  model_info *clash = g_hash_table_lookup (map->hash, info->id);
  g_warn_if_fail (clash == NULL);
  g_hash_table_insert (map->hash, info->id, info);
}

model_info *
model_map_find_by_id (model_map *map, char *id)
{
  return g_hash_table_lookup (map->hash, id);
}
