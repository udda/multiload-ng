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


bool
ml_infofile_has_contents (const char *path, const char *contents, bool case_sensitive)
{
	if_unlikely (path == NULL || contents == NULL)
		return false;

	bool result;

	size_t n = strlen (contents);
	if (n == 0)
		return false;

	FILE *f = fopen (path, "r");
	if_unlikely (f == NULL)
		return false;

	char *buf = (char*)malloc (n);

	if (fread (buf, 1, n, f) != n) {
		result = false;
	} else {
		if (case_sensitive)
			result = (strncmp (buf, contents, n) == 0);
		else
			result = (strncasecmp(buf, contents, n) == 0);
	}

	free (buf);
	fclose (f);
	return result;
}

bool
ml_infofile_read_string_s (const char *path, char *buf, const size_t bufsize, size_t *len_ptr)
{
	if_unlikely (path == NULL || buf == NULL || bufsize < 1)
		return false;

	FILE *f = fopen (path, "r");
	if_unlikely (f == NULL)
		return false;

	size_t s = fread (buf, 1, bufsize-1, f);
	fclose(f);

	if (s < 1 || s >= bufsize)
		return false;

	while (s>0 && buf[s-1] == '\n')
		s--;

	buf[s] = '\0';

	if (len_ptr != NULL)
		*len_ptr = s;

	return true;
}

bool
ml_infofile_read_string (const char *path, char **out, size_t *len_ptr)
{
	if_unlikely (path == NULL || out == NULL)
		return false;

	size_t fsize = ml_file_size (path);
	if (fsize == 0)
		return false;

	*out = malloc (fsize);
	if (!ml_infofile_read_string_s (path, *out, fsize, len_ptr))
		return false;

	return true;
}

bool
ml_infofile_read_int32 (const char *path, int32_t *out)
{
	int64_t v;

	if (!ml_infofile_read_int64 (path, &v))
		return false;

	*out = (int32_t)v;

	return true;
}

bool
ml_infofile_read_uint32 (const char *path, uint32_t *out)
{
	uint64_t v;

	if (!ml_infofile_read_uint64 (path, &v))
		return false;

	*out = (uint32_t)v;

	return true;
}

bool
ml_infofile_read_hex32 (const char *path, uint32_t *out)
{
	uint64_t v;

	if (!ml_infofile_read_hex64 (path, &v))
		return false;

	*out = (uint32_t)v;

	return true;
}

bool
ml_infofile_read_int64 (const char *path, int64_t *out)
{
	if_unlikely (path == NULL || out == NULL)
		return false;

	char buf[30];
	char *endptr;
	if (!ml_infofile_read_string_s (path, buf, sizeof(buf), NULL))
		return false;

	*out = (int64_t)ml_ascii_strtoll (buf, &endptr, 10);
	if_unlikely (buf == endptr)
		return false;

	return true;
}

bool
ml_infofile_read_uint64 (const char *path, uint64_t *out)
{
	if_unlikely (path == NULL || out == NULL)
		return false;

	char buf[30];
	char *endptr;
	if (!ml_infofile_read_string_s (path, buf, sizeof(buf), NULL))
		return false;

	*out = (uint64_t)ml_ascii_strtoull (buf, &endptr, 10);
	if_unlikely (buf == endptr)
		return false;

	return true;
}

bool
ml_infofile_read_hex64 (const char *path, uint64_t *out)
{
	if_unlikely (path == NULL || out == NULL)
		return false;

	char buf[30];
	char *endptr;
	if (!ml_infofile_read_string_s (path, buf, sizeof(buf), NULL))
		return false;

	*out = (uint64_t)ml_ascii_strtoull (buf, &endptr, 16);
	if_unlikely (buf == endptr)
		return false;

	return true;
}

bool
ml_infofile_read_double (const char *path, double *out, double scale)
{
	if_unlikely (path == NULL || out == NULL || scale <= 0)
		return false;

	char buf[50];
	char *endptr;
	if (!ml_infofile_read_string_s(path, buf, sizeof(buf), NULL))
		return false;

	*out = ml_ascii_strtod (buf, &endptr);
	if_unlikely (buf == endptr)
		return false;

	if (scale > 0)
		*out /= scale;

	return true;
}

bool
ml_infofile_read_key_string_nth_s (const char *path, const char *key, size_t index, char *buf, size_t bufsize, size_t *len_ptr)
{
	/* returns the 'index'-th result (0-based) */
	if_unlikely (path == NULL || key == NULL || buf == NULL || bufsize < 1)
		return false;

	bool found = false;
	char *line = NULL;
	size_t n = 0;

	FILE *f = fopen(path, "r");
	if_unlikely (f == NULL)
		return false;

	size_t keylen = strlen(key);
	ssize_t linelen;

	while ((linelen = getline(&line, &n, f)) >= 0) {
		if ((size_t)linelen <= keylen)
			continue;

		if (strncmp(line, key, keylen) == 0) {

			// search for ':' character
			char *pch = strchr(line, ':');
			if (pch == NULL)
				continue;

			// check whether skip current entry and search for next one
			if (index == 0) {
				found = true;
			} else {
				index--;
				continue;
			}

			// move pch forward to the value
			while (pch-line<linelen) {
				if (*pch==':' || ml_isspace(*pch))
					pch++;
				else
					break;
			}

			if (pch-line == linelen) {
				// reached end of line - set buf to an empty string
				buf[0] = '\0';
				if (len_ptr != NULL)
					*len_ptr = 0;
				break;
			}

			// copy value into buffer
			bufsize = MIN (bufsize, (size_t)linelen-(pch-line));
			strncpy(buf, pch, bufsize);
			if (len_ptr != NULL)
				*len_ptr = bufsize;

			// remove trailing newline
			if (buf[bufsize-1] == '\n')
				buf[bufsize-1] = '\0';
		}
	}

	free(line);
	fclose(f);

	return found;
}

bool
ml_infofile_read_key_string_s (const char *path, const char *key, char *buf, size_t bufsize, size_t *len_ptr)
{
	return ml_infofile_read_key_string_nth_s (path, key, 0, buf, bufsize, len_ptr);
}

bool
ml_infofile_read_key_int32 (const char *path, const char *key, int32_t *out)
{
	int64_t v;

	if (!ml_infofile_read_key_int64 (path, key, &v))
		return false;

	*out = (int32_t)v;

	return true;
}

bool
ml_infofile_read_key_uint32 (const char *path, const char *key, uint32_t *out)
{
	uint64_t v;

	if (!ml_infofile_read_key_uint64 (path, key, &v))
		return false;

	*out = (uint32_t)v;

	return true;
}

bool
ml_infofile_read_key_hex32 (const char *path, const char *key, uint32_t *out)
{
	uint64_t v;

	if (!ml_infofile_read_key_hex64 (path, key, &v))
		return false;

	*out = (uint32_t)v;

	return true;
}

bool
ml_infofile_read_key_int64 (const char *path, const char *key, int64_t *out)
{
	if_unlikely (path == NULL || key == NULL || out == NULL)
		return false;

	char buf[30];
	char *endptr;
	if (!ml_infofile_read_key_string_s(path, key, buf, sizeof(buf), NULL))
		return false;

	*out = (int64_t)ml_ascii_strtoll (buf, &endptr, 10);
	if_unlikely (buf == endptr)
		return false;

	return true;
}

bool
ml_infofile_read_key_uint64 (const char *path, const char *key, uint64_t *out)
{
	if_unlikely (path == NULL || key == NULL || out == NULL)
		return false;

	char buf[30];
	char *endptr;
	if (!ml_infofile_read_key_string_s(path, key, buf, sizeof(buf), NULL))
		return false;

	*out = (uint64_t)ml_ascii_strtoull (buf, &endptr, 10);
	if_unlikely (buf == endptr)
		return false;

	return true;
}

bool
ml_infofile_read_key_hex64 (const char *path, const char *key, uint64_t *out)
{
	if_unlikely (path == NULL || key == NULL || out == NULL)
		return false;

	char buf[30];
	char *endptr;
	if (!ml_infofile_read_key_string_s(path, key, buf, sizeof(buf), NULL))
		return false;

	*out = (uint64_t)ml_ascii_strtoull (buf, &endptr, 16);
	if_unlikely (buf == endptr)
		return false;

	return true;
}

bool
ml_infofile_read_key_double (const char *path, const char *key, double *out, double scale)
{
	if_unlikely (path == NULL || key == NULL || out == NULL)
		return false;

	char buf[50];
	char *endptr;
	if (!ml_infofile_read_key_string_s(path, key, buf, sizeof(buf), NULL))
		return false;

	*out = ml_ascii_strtod (buf, &endptr);
	if_unlikely (buf == endptr)
		return false;

	if (scale > 0)
		*out /= scale;

	return true;
}


int
ml_infofile_read_keys (const char *path, const MlInfofileMappingEntry *entries, int count)
{
	if_unlikely (path == NULL || entries == NULL || count<1)
		return false;

	int ret = 0;

	char *line = NULL;
	size_t n = 0;

	char *pch, *endptr;
	size_t len;
	int linelen;

	FILE *f = fopen(path, "r");
	if_unlikely (f == NULL)
		return -1;

	while ((linelen=getline(&line, &n, f)) >= 0) {
		for (int i = 0; i < count; i++) {
			len = strlen(entries[i].key);

			if (strncmp(line, entries[i].key, len) == 0) {
				pch = line+len;

				while (pch-line<linelen) {
					if (*pch==':' || ml_isspace(*pch))
						pch++;
					else
						break;
				}

				switch (entries[i].type) {
					case 's': // entries[i].address is a char**
						(*((char**)entries[i].address)) = ml_strdup (pch);

						for (pch = (*((char**)entries[i].address)); *pch != '\0'; pch++) {
							if (*pch == '\n') {
								*pch = '\0';
								break;
							}
						}
						ret++;
						break;

					case 'i': // entries[i].address is a int32_t*
						(*((int32_t*)entries[i].address)) = (int32_t)ml_ascii_strtoll (pch, &endptr, 10);
						if_likely (pch != endptr)
							ret++;
						break;

					case 'u': // entries[i].address is a uint32_t*
						(*((uint32_t*)entries[i].address)) = (uint32_t)ml_ascii_strtoull (pch, &endptr, 10);
						if_likely (pch != endptr)
							ret++;
						break;

					case 'x': // entries[i].address is a uint32_t* (hex)
						(*((uint32_t*)entries[i].address)) = (uint32_t)ml_ascii_strtoull (pch, &endptr, 16);
						if_likely (pch != endptr)
							ret++;
						break;

					case 'I': // entries[i].address is a int64_t*
						(*((int64_t*)entries[i].address)) = (int64_t)ml_ascii_strtoll (pch, &endptr, 10);
						if_likely (pch != endptr)
							ret++;
						break;

					case 'U': // entries[i].address is a uint64_t*
						(*((uint64_t*)entries[i].address)) = (uint64_t)ml_ascii_strtoull (pch, &endptr, 10);
						if_likely (pch != endptr)
							ret++;
						break;

					case 'X': // entries[i].address is a uint64_t* (hex)
						(*((uint64_t*)entries[i].address)) = (uint64_t)ml_ascii_strtoull (pch, &endptr, 16);
						if_likely (pch != endptr)
							ret++;
						break;

					case 'd': // entries[i].address is a double*
						(*((double*)entries[i].address)) = (double)ml_ascii_strtod (pch, &endptr);
						if_likely (pch != endptr)
							ret++;
						break;

					default:
						ml_bug ("Unknown key type '%c'", entries[i].type);
						break;
				}
			}
		}
	}

	free(line);
	fclose(f);

	return ret;
}


int
ml_infofile_count_key_values (const char *path, const char *key)
{
	if_unlikely (path == NULL || key == NULL)
		return 0;

	int ret = 0;

	char *line = NULL;
	size_t n = 0;

	FILE *f = fopen(path, "r");
	if_unlikely (f == NULL)
		return false;

	size_t keylen = strlen(key);
	ssize_t linelen;

	while ((linelen = getline(&line, &n, f)) >= 0) {
		if ((size_t)linelen <= keylen)
			continue;

		if (strncmp(line, key, keylen) == 0)
			ret++;
	}

	free(line);
	fclose(f);

	return ret;
}
