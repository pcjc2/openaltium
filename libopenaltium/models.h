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

typedef struct model_info model_info;
/* PUBLIC FOR NOW */
struct model_info {
  char *id;
  double rotx;
  double roty;
  double rotz;
  int32_t dx;
  int32_t dy;
  int32_t dz;
  int checksum;
  bool embed;
  char *filename;
};


model_info *model_info_new_from_parameters (const parameter_list *parameters);
void model_info_free (model_info *info);

typedef struct model_map  model_map;
model_map *model_map_new ();
void model_map_insert (model_map *map, model_info *info);
void model_map_free (model_map *map);
model_info *model_map_find_by_id (model_map *map, char *id);
