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

typedef struct {
  char *data;
  unsigned int length;
  unsigned int cursor;
} file_content;

int content_check_available (file_content *content, unsigned int length);
int content_get_uint32 (file_content *content, uint32_t *data);
int content_get_int32 (file_content *content, int32_t *data);
//int content_get_uint16 (file_content *content, uint32_t *data);
int content_get_byte (file_content *content, uint8_t *data);
int content_get_double (file_content *content, double *data);

char *content_get_n_chars (file_content *content, unsigned int n_chars);
char *content_get_n_wchars (file_content *content, unsigned int n_chars);
int content_skip_bytes (file_content *content, unsigned int n_bytes);
char *content_get_length_multi_prefixed_string (file_content *content);
char *content_get_length_dword_prefixed_string (file_content *content);
