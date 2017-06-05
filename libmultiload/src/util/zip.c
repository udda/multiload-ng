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
ml_zip_add_buffer (mz_zip_archive *zip, const char *filename, void *buf, size_t buflen, unsigned compression_level)
{
	if (zip == NULL || filename == NULL || compression_level > 9)
		return false;

	if (buf == NULL || buflen == 0)
		return true;

	bool result = mz_zip_writer_add_mem (zip, filename, buf, buflen, compression_level);
	if (!result)
		ml_warning ("Cannot add %s to zip: %s", filename, ml_zip_get_error_string(zip));

	return result;
}

bool
ml_zip_add_grow_buffer (mz_zip_archive *zip, const char *filename, MlGrowBuffer *gbuf, unsigned compression_level)
{
	if (zip == NULL || filename == NULL || compression_level > 9)
		return false;

	if (gbuf == NULL || ml_grow_buffer_get_length(gbuf) == 0)
		return true;

	bool result = mz_zip_writer_add_mem (zip, filename, ml_grow_buffer_get_data (gbuf), ml_grow_buffer_get_length (gbuf), compression_level);
	if (!result)
		ml_warning ("Cannot add %s to zip: %s", filename, ml_zip_get_error_string(zip));

	return result;
}

bool
ml_zip_add_string (mz_zip_archive *zip, const char *filename, const char *str, unsigned compression_level)
{
	if (zip == NULL || filename == NULL || compression_level > 9)
		return false;

	if (str == NULL)
		return true;

	bool result = mz_zip_writer_add_mem (zip, filename, str, strlen(str), compression_level);
	if (!result)
		ml_warning ("Cannot add %s to zip: %s", filename, ml_zip_get_error_string(zip));

	return result;
}

bool
ml_zip_add_directory (mz_zip_archive *zip, const char *dirname)
{
	if (zip == NULL || dirname == NULL)
		return false;

	// if not present, add terminating '/' to path, in order
	// to make Miniz recognize this filename as a directory
	char path[PATH_MAX];
	strncpy (path, dirname, PATH_MAX);
	size_t len = strlen (dirname);
	if (path[len-1] != '/') {
		path[len] = '/';
		path[len+1] = '\0';
	}

	// write entry
	bool result = mz_zip_writer_add_mem (zip, path, NULL, 0, MZ_NO_COMPRESSION);
	if (!result)
		ml_warning ("Cannot create %s directory into zip: %s", dirname, ml_zip_get_error_string(zip));

	return result;
}

bool
ml_zip_add_file (mz_zip_archive *zip, const char *filename, const char *src_filename, unsigned compression_level)
{
	if (zip == NULL || filename == NULL || compression_level > 9)
		return false;

	MlGrowBuffer *gbuf = ml_grow_buffer_new_from_file (src_filename);
	if (gbuf == NULL) {
		ml_warning ("Cannot add %s to zip: source file %s is not available", filename, src_filename);
		return false;
	}

	bool ret = ml_zip_add_grow_buffer (zip, filename, gbuf, compression_level);
	ml_grow_buffer_destroy (gbuf, true);

	return ret;
}

const char *
ml_zip_get_error_string (mz_zip_archive *zip)
{
	mz_zip_error err = mz_zip_get_last_error(zip);
	return mz_zip_get_error_string(err);
}
