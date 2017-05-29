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

#ifndef ML_HEADER__UTIL_ZIP_H__INCLUDED
#define ML_HEADER__UTIL_ZIP_H__INCLUDED
ML_HEADER_BEGIN


bool
ml_zip_add_buffer (mz_zip_archive *zip, const char *filename, void *buf, size_t buflen, unsigned compression_level);

bool
ml_zip_add_grow_buffer (mz_zip_archive *zip, const char *filename, MlGrowBuffer *gbuf, unsigned compression_level);

bool
ml_zip_add_string (mz_zip_archive *zip, const char *filename, const char *str, unsigned compression_level);

bool
ml_zip_add_directory (mz_zip_archive *zip, const char *dirname);

bool
ml_zip_add_file (mz_zip_archive *zip, const char *filename, const char *src_filename, unsigned compression_level);

const char *
ml_zip_get_error_string (mz_zip_archive *zip);


ML_HEADER_END
#endif /* ML_HEADER__UTIL_ZIP_H__INCLUDED */
