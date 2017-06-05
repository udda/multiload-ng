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

#ifndef ML_HEADER__UTIL_INFOFILE_H__INCLUDED
#define ML_HEADER__UTIL_INFOFILE_H__INCLUDED
ML_HEADER_BEGIN


typedef struct {
	char *	key;
	char	type; // one of 's', 'i', 'u', 'x', 'I', 'U', 'X', 'd'
	mlPointer	address;
} MlInfofileMappingEntry;


bool
ml_infofile_has_contents (const char *path, const char *contents, bool case_sensitive)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_string_s (const char *path, char *buf, const size_t bufsize, size_t *len_ptr)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_string (const char *path, char **out, size_t *len_ptr)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_int32 (const char *path, int32_t *out)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_uint32 (const char *path, uint32_t *out)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_hex32 (const char *path, uint32_t *out)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_int64 (const char *path, int64_t *out)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_uint64 (const char *path, uint64_t *out)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_hex64 (const char *path, uint64_t *out)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_double (const char *path, double *out, double scale)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;


bool
ml_infofile_read_key_string_nth_s (const char *path, const char *key, size_t index, char *buf, size_t bufsize, size_t *len_ptr)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_key_string_s (const char *path, const char *key, char *buf, size_t bufsize, size_t *len_ptr)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_key_int32 (const char *path, const char *key, int32_t *out)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_key_uint32 (const char *path, const char *key, uint32_t *out)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_key_hex32 (const char *path, const char *key, uint32_t *out)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_key_int64 (const char *path, const char *key, int64_t *out)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_key_uint64 (const char *path, const char *key, uint64_t *out)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_key_hex64 (const char *path, const char *key, uint64_t *out)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

bool
ml_infofile_read_key_double (const char *path, const char *key, double *out, double scale)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;


int
ml_infofile_read_keys (const char *path, const MlInfofileMappingEntry *entries, int count)
ML_FN_HOT;

int
ml_infofile_count_key_values (const char *path, const char *key)
ML_FN_HOT;


ML_HEADER_END
#endif /* ML_HEADER__UTIL_INFOFILE_H__INCLUDED */
