/*
 * Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of Multiload-ng.
 *
 * Multiload-ng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Multiload-ng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <multiload.h>


struct _MlGrowBuffer {
	uint8_t *data;
	size_t len;
	size_t allocated_size;
};


MlGrowBuffer *
ml_grow_buffer_new (size_t starting_size)
{
	if_unlikely (starting_size < 1)
		starting_size = 1;

	MlGrowBuffer *gbuf = ml_new (MlGrowBuffer);

	gbuf->data = ml_new_n (uint8_t, starting_size);
	gbuf->len = 0;
	gbuf->allocated_size = starting_size;

	return gbuf;
}

MlGrowBuffer *
ml_grow_buffer_new_from_buffer (mlPointer buf, size_t len)
{
	MlGrowBuffer *gbuf = ml_grow_buffer_new (len);
	if (gbuf == NULL)
		return NULL;

	ml_grow_buffer_append (gbuf, buf, len);
	return gbuf;
}

MlGrowBuffer *
ml_grow_buffer_new_from_string (char *str)
{
	MlGrowBuffer *gbuf = ml_grow_buffer_new (0);
	if (gbuf == NULL)
		return NULL;

	ml_grow_buffer_append_string (gbuf, str);
	return gbuf;
}

MlGrowBuffer *
ml_grow_buffer_new_from_file (const char *path)
{
	MlGrowBuffer *gbuf = ml_grow_buffer_new (0);
	if (gbuf == NULL)
		return NULL;

	if (!ml_grow_buffer_append_file (gbuf, path)) {
		ml_grow_buffer_destroy (gbuf, true);
		return NULL;
	}

	return gbuf;
}

mlPointer
ml_grow_buffer_destroy (MlGrowBuffer *gbuf, bool free_data)
{
	if_unlikely (gbuf == NULL)
		return NULL;

	uint8_t *data;

	if (free_data || gbuf->len == 0) {
		free (gbuf->data);
		data = NULL;
	} else {
		data = ml_renew (uint8_t, gbuf->data, gbuf->len);
	}

	free (gbuf);

	return data;
}

size_t
ml_grow_buffer_sizeof (MlGrowBuffer *gbuf)
{
	if_unlikely (gbuf == NULL)
		return 0;

	size_t size = sizeof (MlGrowBuffer);
	size += sizeof (uint8_t) * gbuf->allocated_size;

	return size;
}

void
ml_cleanup_MlGrowBuffer (MlGrowBuffer **p)
{
	if_likely (p != NULL) {
		ml_grow_buffer_destroy (*p, true);
		*p = NULL;
	}
}

void
ml_grow_buffer_rewind (MlGrowBuffer *gbuf)
{
	// "empty" buffer without deallocating (much more efficient than a new/destroy cycle)
	if_unlikely (gbuf == NULL || gbuf->len == 0)
		return;

	gbuf->data[0] = '\0';
	gbuf->len = 0;
}

mlPointer
ml_grow_buffer_get_data (MlGrowBuffer *gbuf)
{
	if_unlikely (gbuf == NULL)
		return NULL;

	return (mlPointer)gbuf->data;
}

size_t
ml_grow_buffer_get_length (MlGrowBuffer *gbuf)
{
	if_unlikely (gbuf == NULL)
		return 0;

	return gbuf->len;
}

void
ml_grow_buffer_append (MlGrowBuffer *gbuf, mlPointer buf, size_t len)
{
	if_unlikely (gbuf == NULL || buf == NULL || len < 1)
		return;

	if (gbuf->len + len + 1 > gbuf->allocated_size) { // the '+ 1' is for the terminating NULL element, see below
		ml_debug ("Expanding buffer from %zu to %zu", gbuf->allocated_size, gbuf->allocated_size + len * 2);
		gbuf->allocated_size += len * 2; // some extra space at the end
		gbuf->data = ml_renew (uint8_t, gbuf->data, gbuf->allocated_size);
	}

	memcpy (gbuf->data+gbuf->len, buf, len);
	gbuf->len += len;

	/* Here we append a terminating NULL element at the end of the buffer.
	 * Functions that use MlGrowBuffer for binary data will ignore it, as they
	 * will read exactly gbuf->len bytes, functions that use MlGrowBuffer for
	 * storing strings will get a zero-terminated string at no additional cost */
	gbuf->data[gbuf->len] = '\0';
}

void
ml_grow_buffer_append_string (MlGrowBuffer *gbuf, char *str)
{
	if_unlikely (gbuf == NULL || str == NULL || str[0] == '\0')
		return;

	ml_grow_buffer_append (gbuf, (mlPointer)str, strlen(str));
}

void
ml_grow_buffer_append_printf (MlGrowBuffer *gbuf, const char *fmt, ...)
{
	if_unlikely (gbuf == NULL)
		return;

	va_list va;
	va_start (va, fmt);
	ml_grow_buffer_append_vprintf (gbuf, fmt, va);
	va_end (va);
}

void
ml_grow_buffer_append_vprintf (MlGrowBuffer *gbuf, const char *fmt, va_list va)
{
	if_unlikely (gbuf == NULL)
		return;

	char buf[ML_PRINTF_STATIC_BUFFER_SIZE]; // see global.h to learn about buffer size

	int n = vsnprintf (buf, sizeof(buf), fmt, va);

	if (n > 0)
		ml_grow_buffer_append (gbuf, buf, n);
}

bool
ml_grow_buffer_append_file (MlGrowBuffer *gbuf, const char *path)
{
	if_unlikely (gbuf == NULL || path == NULL || path[0] == '\0')
		return false;

	char buf[ML_FREAD_BUFFER_SIZE];
	size_t r;

	FILE *f = fopen (path, "r");
	if (f == NULL)
		return false;

	while ((r = fread (buf, 1, sizeof(buf), f)) > 0)
		ml_grow_buffer_append (gbuf, buf, r);

	fclose (f);
	return true;
}

void
ml_grow_buffer_append_grow_buffer (MlGrowBuffer *gbuf, MlGrowBuffer *append)
{
	if_unlikely (gbuf == NULL || append == NULL)
		return;

	ml_grow_buffer_append (gbuf, append->data, append->len);
}
