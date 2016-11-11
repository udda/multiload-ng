/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *                    The Free Software Foundation
 *
 * This file is part of multiload-ng.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "info-file.h"

#include <string.h>

FILE*
info_file_required_fopen (const gchar *path, const gchar *mode)
{
	FILE *f = fopen (path, mode);
	g_assert (f != NULL);
	return f;
}

gboolean
info_file_exists (const gchar *path)
{
	return g_file_test(path, G_FILE_TEST_EXISTS);
}

gboolean
info_file_has_contents (const gchar *path, const gchar *contents)
{
	if (path == NULL || contents == NULL)
		return FALSE;

	gboolean result;

	FILE *f = fopen(path, "r");
	if (f == NULL)
		return FALSE;

	size_t n = strlen(contents);
	gchar *buf = (gchar*)malloc(n);

	size_t s = fread(buf, 1, n, f);

	if (s != n)
		result = FALSE;
	else if (strncmp(buf, contents, n) != 0)
		result = FALSE;
	else
		result = TRUE;

	g_free(buf);
	fclose(f);
	return result;
}

gboolean
info_file_read_string_s (const gchar *path, gchar *buf, const size_t bufsize, size_t *length)
{
	if (path == NULL || buf == NULL || bufsize < 1)
		return FALSE;

	FILE *f = fopen(path, "r");
	if (!f)
		return FALSE;

	size_t s = fread(buf, 1, bufsize-1, f);
	fclose(f);

	if (s < 1 || s >= bufsize)
		return FALSE;

	while (s>0 && buf[s-1] == '\n')
		s--;

	buf[s] = '\0';

	if (length != NULL)
		*length = s;

	return TRUE;
}

gboolean
info_file_read_string (const gchar *path, gchar **out, size_t *length)
{
	if (path == NULL || out == NULL)
		return FALSE;

	size_t n;

	if (!g_file_get_contents (path, out, &n, NULL))
		return FALSE;

	while (n>0 && (*out)[n-1] == '\n') {
		(*out)[--n] = '\0';
	}

	if (length != NULL)
		*length = n;

	return TRUE;
}

gboolean
info_file_read_int64 (const gchar *path, gint64 *out)
{
	if (path == NULL || out == NULL)
		return FALSE;

	gchar buf[30];
	gchar *endptr;
	if (!info_file_read_string_s(path, buf, sizeof(buf), NULL))
		return FALSE;

	*out = g_ascii_strtoll (buf, &endptr, 10);
	if (buf == endptr)
		return FALSE;

	return TRUE;
}

gboolean
info_file_read_uint64 (const gchar *path, guint64 *out)
{
	if (path == NULL || out == NULL)
		return FALSE;

	gchar buf[30];
	gchar *endptr;
	if (!info_file_read_string_s(path, buf, sizeof(buf), NULL))
		return FALSE;

	*out = g_ascii_strtoull (buf, &endptr, 10);
	if (buf == endptr)
		return FALSE;

	return TRUE;
}

gboolean
info_file_read_hex64 (const gchar *path, guint64 *out)
{
	if (path == NULL || out == NULL)
		return FALSE;

	gchar buf[30];
	gchar *endptr;
	if (!info_file_read_string_s(path, buf, sizeof(buf), NULL))
		return FALSE;

	*out = g_ascii_strtoull (buf, &endptr, 16);
	if (buf == endptr)
		return FALSE;

	return TRUE;
}

gboolean
info_file_read_double (const gchar *path, gdouble *out, gdouble scale)
{
	if (path == NULL || out == NULL || scale <= 0)
		return FALSE;

	gchar buf[50];
	gchar *endptr;
	if (!info_file_read_string_s(path, buf, sizeof(buf), NULL))
		return FALSE;

	*out = g_ascii_strtod (buf, &endptr);
	if (buf == endptr)
		return FALSE;

	if (scale > 0)
		*out /= scale;

	return TRUE;
}

gboolean
info_file_read_key_string_s (const gchar *path, const gchar *key, gchar *buf, size_t bufsize, size_t *length)
{
	if (path == NULL || key == NULL || buf == NULL || bufsize < 1)
		return FALSE;

	gboolean found = FALSE;
	gchar *line = NULL;
	size_t n = 0;

	FILE *f = fopen(path, "r");
	if (f == NULL)
		return FALSE;

	size_t keylen = strlen(key);
	ssize_t linelen;

	while ((linelen = getline(&line, &n, f)) >= 0) {
		if (linelen <= keylen)
			continue;

		if (strncmp(line, key, keylen) == 0) {
			gchar *pch = strchr(line, ':');

			if (pch == NULL)
				continue;

			while (pch-line<linelen) {
				if (*pch==':' || g_ascii_isspace(*pch))
					pch++;
				else
					break;
			}

			if (pch-line == linelen) {
				found = FALSE;
				break;
			}
			if (bufsize > linelen-(pch-line))
				bufsize = linelen-(pch-line);

			strncpy(buf, pch, bufsize);
			found = TRUE;

			// remove newline
			if (buf[bufsize-1] == '\n')
				buf[bufsize-1] = '\0';

			if (length != NULL)
				*length = bufsize;
		}
	}

	g_free(line);
	fclose(f);

	return found;
}

gboolean
info_file_read_key_int64 (const gchar *path, const gchar *key, gint64 *out)
{
	if (path == NULL || key == NULL || out == NULL)
		return FALSE;

	gchar buf[30];
	gchar *endptr;
	if (!info_file_read_key_string_s(path, key, buf, sizeof(buf), NULL))
		return FALSE;

	*out = g_ascii_strtoll (buf, &endptr, 10);
	if (buf == endptr)
		return FALSE;

	return TRUE;
}

gboolean
info_file_read_key_uint64 (const gchar *path, const gchar *key, guint64 *out)
{
	if (path == NULL || key == NULL || out == NULL)
		return FALSE;

	gchar buf[30];
	gchar *endptr;
	if (!info_file_read_key_string_s(path, key, buf, sizeof(buf), NULL))
		return FALSE;

	*out = g_ascii_strtoull (buf, &endptr, 10);
	if (buf == endptr)
		return FALSE;

	return TRUE;
}

gboolean
info_file_read_key_hex64 (const gchar *path, const gchar *key, guint64 *out)
{
	if (path == NULL || key == NULL || out == NULL)
		return FALSE;

	gchar buf[30];
	gchar *endptr;
	if (!info_file_read_key_string_s(path, key, buf, sizeof(buf), NULL))
		return FALSE;

	*out = g_ascii_strtoull (buf, &endptr, 16);
	if (buf == endptr)
		return FALSE;

	return TRUE;
}

gboolean
info_file_read_key_double (const gchar *path, const gchar *key, gdouble *out, gdouble scale)
{
	if (path == NULL || key == NULL || out == NULL)
		return FALSE;

	gchar buf[50];
	gchar *endptr;
	if (!info_file_read_key_string_s(path, key, buf, sizeof(buf), NULL))
		return FALSE;

	*out = g_ascii_strtod (buf, &endptr);
	if (buf == endptr)
		return FALSE;

	if (scale > 0)
		*out /= scale;

	return TRUE;
}


gint
info_file_read_keys (const gchar *path, const InfoFileMappingEntry *entries, gint count)
{
	if (path == NULL || entries == NULL || count<1)
		return FALSE;

	gint ret = 0;

	gchar *line = NULL;
	size_t n = 0;

	gchar *pch, *endptr;
	size_t len;
	int linelen;
	guint i;

	FILE *f = fopen(path, "r");
	if (f == NULL)
		return -1;

	while ((linelen=getline(&line, &n, f)) >= 0) {
		for (i=0; i<count; i++) {
			len = strlen(entries[i].key);

			if (strncmp(line, entries[i].key, len) == 0) {
				pch = line+len;

				while (pch-line<linelen) {
					if (*pch==':' || g_ascii_isspace(*pch))
						pch++;
					else
						break;
				}

				switch (entries[i].type) {
					case 's': // entries[i].address is a gchar**
						(*((gchar**)entries[i].address)) = g_strdup(pch);

						for (pch = (*((gchar**)entries[i].address)); *pch != '\0'; pch++) {
							if (*pch == '\n') {
								*pch = '\0';
								break;
							}
						}
						break;

					case 'i': // entries[i].address is a gint64*
						(*((gint64*)entries[i].address)) = g_ascii_strtoll (pch, &endptr, 10);
						if (pch != endptr)
							ret++;
						break;

					case 'u': // entries[i].address is a guint64*
						(*((guint64*)entries[i].address)) = g_ascii_strtoull (pch, &endptr, 10);
						if (pch != endptr)
							ret++;
						break;

					case 'x': // entries[i].address is a guint64* (hex)
						(*((guint64*)entries[i].address)) = g_ascii_strtoull (pch, &endptr, 16);
						if (pch != endptr)
							ret++;
						break;

					case 'd': // entries[i].address is a gdouble*
						(*((gdouble*)entries[i].address)) = g_ascii_strtod (pch, &endptr);
						if (pch != endptr)
							ret++;
						break;

					default:
						g_assert_not_reached();
				}
			}
		}
	}

	g_free(line);
	fclose(f);

	return ret;
}


guint
info_file_count_key_values (const gchar *path, const gchar *key)
{
	if (path == NULL || key == NULL)
		return 0;

	gint ret = 0;

	gchar *line = NULL;
	size_t n = 0;

	FILE *f = fopen(path, "r");
	if (f == NULL)
		return FALSE;

	size_t keylen = strlen(key);
	ssize_t linelen;

	while ((linelen = getline(&line, &n, f)) >= 0) {
		if (linelen <= keylen)
			continue;

		if (strncmp(line, key, keylen) == 0)
			ret++;
	}

	g_free(line);
	fclose(f);

	return ret;
}
