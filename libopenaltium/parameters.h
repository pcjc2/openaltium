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

typedef struct parameter_list parameter_list;

parameter_list *parameter_list_new_from_string (const char *string);
void parameter_list_free (parameter_list *list);
int32_t parameter_list_get_dimension (const parameter_list *list, const char *name);
double parameter_list_get_double (const parameter_list *list, const char *name);
unsigned int parameter_list_get_unsigned_int (const parameter_list *list, const char *name);
int parameter_list_get_int (const parameter_list *list, const char *name);
bool parameter_list_get_bool (const parameter_list *list, const char *name);
char *parameter_list_get_string (const parameter_list *list, const char *name);
