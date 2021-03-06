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
#include <glib.h>
#include <stdio.h>

#include "content-parser.h"

int
content_check_available (file_content *content, unsigned int length)
{
  return (content->cursor + length > content->length) ? 0 : 1;
}

/* FIXME: These need to read from the file as little endian, the
 *        implementation only works in Little Endian computers.
 */
#define CONTENT_GET_TYPE(name, type) \
int \
content_get_##name (file_content *content, type *data) \
{ \
  g_return_val_if_fail (content_check_available (content, sizeof (type)), 0); \
  *data = *(type*)&content->data[content->cursor]; \
  content->cursor += sizeof (type); \
  return 1; \
}

CONTENT_GET_TYPE(uint32, uint32_t)
CONTENT_GET_TYPE(int32, int32_t)
CONTENT_GET_TYPE(uint16, uint16_t)
CONTENT_GET_TYPE(int16, int16_t)
CONTENT_GET_TYPE(byte, uint8_t)
CONTENT_GET_TYPE(double, double)


char *
content_get_n_chars (file_content *content, unsigned int n_chars)
{
  char *data;
  g_return_val_if_fail (content_check_available (content, n_chars), NULL);
  data = g_strndup (&content->data[content->cursor], n_chars);
  content->cursor += n_chars;
  return data;
}


char *
content_get_n_wchars (file_content *content, unsigned int n_chars)
{
  char *data;
  g_return_val_if_fail (content_check_available (content, 2 * n_chars), NULL);
  /* FIXME: Add error handling for this next call */
  data = g_utf16_to_utf8 ((gunichar2 *)&content->data[content->cursor], n_chars, NULL, NULL, NULL);
  content->cursor += 2 * n_chars;
  return data;
}


int
content_skip_bytes (file_content *content, unsigned int n_bytes)
{
  int i;
  g_return_val_if_fail (content_check_available (content, n_bytes), 0);
  for (i = 0; i < n_bytes; i++) {
    printf ("Skipped byte %i\n", content->data[content->cursor + i]);
  }
  content->cursor += n_bytes;
  return 1;
}


char *
content_get_length_multi_prefixed_string (file_content *content)
{
  uint32_t txt_block_length;
  uint8_t txt_length;
  char *string;

  /* Text block skip length */
  if (!content_get_uint32 (content, &txt_block_length))
    return NULL;

#if 0
  if (txt_block_length == 0) {
    printf ("0 LEN TXT!!!!\n");
    return NULL; //g_strdup(""); /* empty string? */
  }
#endif

  if (!content_get_byte (content, &txt_length))
    return NULL;

  fflush (stdout);
  if (txt_block_length != 1 + txt_length) //exit (-1);
    g_warning ("txt_block_length = %i\n, 1 + txt_length = %i\n", txt_block_length, 1 + txt_length);

  if (txt_block_length == 0 && txt_length == 0)
    return g_strdup(""); /* empty string? */

  //g_return_val_if_fail (txt_block_length == 1 + txt_length, NULL);
//REINSTATE!  g_assert (txt_block_length == 1 + txt_length);

  string = content_get_n_chars (content, txt_length);
  return string;
}


char *
content_get_length_dword_prefixed_string (file_content *content)
{
  uint32_t txt_block_length;
  char *string;

  /* Text block skip length */
  if (!content_get_uint32 (content, &txt_block_length))
    return NULL;

  string = content_get_n_chars (content, txt_block_length);
  return string;
}
