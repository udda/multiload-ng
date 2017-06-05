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

#ifndef ML_HEADER__UTIL_STRING_H__INCLUDED
#define ML_HEADER__UTIL_STRING_H__INCLUDED
ML_HEADER_BEGIN
#include "../multiload/shared.h"


/* String arrays (char **) functions */

bool
ml_strv_contains (char **strv, const char *needle)
ML_FN_READ_PROPERTY ML_FN_HOT;

bool
ml_strv_contains_prefix (char **strv, const char *needle)
ML_FN_READ_PROPERTY ML_FN_HOT;

bool
ml_strv_contains_suffix (char **strv, const char *needle)
ML_FN_READ_PROPERTY ML_FN_HOT;

char *
ml_strv_join (char **strv, const char *separator)
ML_FN_HOT;

bool
ml_strv_join_to_grow_buffer (char **strv, const char *separator, MlGrowBuffer *gbuf)
ML_FN_HOT;

size_t
ml_strv_length (char **strv)
ML_FN_READ_PROPERTY ML_FN_HOT;

#define ml_strv_for(strv,index) for (int (index) = 0; (strv)[index] != NULL; (index)++)

size_t
ml_strv_sizeof (char **strv)
ML_FN_SIZEOF;

void
ml_strv_free (char **strv);


/* Strings (char *) functions and macros */

// locale-independent macros
#define ml_isspace(c) ((c)==' ' || (c)=='\t' || (c)=='\n' || (c)=='\v' || (c)=='\f' || (c)=='\r')
#define ml_isdigit(c) ((c)>='0' && (c)<='9')

#define ml_ascii_strtoll(  str, tailptr, base)  strtoll_l  ((str), (tailptr), (base), ML_SHARED_GET (C_locale))
#define ml_ascii_strtoull( str, tailptr, base)  strtoull_l ((str), (tailptr), (base), ML_SHARED_GET (C_locale))
#define ml_ascii_strtod(   str, tailptr)        strtod_l   ((str), (tailptr),         ML_SHARED_GET (C_locale))

#define ml_string_is_null_or_empty(str) ((str) == NULL || *(str) == '\0')
#define ml_string_null_to_empty(str) ((str) == NULL ? "" : (str))

char *
ml_strdup (const char *str)
ML_FN_HOT;

size_t
ml_string_sizeof (char *str)
ML_FN_SIZEOF;

bool
ml_string_equals (const char *str1, const char *str2, bool case_sensitive)
ML_FN_READ_PROPERTY ML_FN_HOT;

bool
ml_string_has_prefix (const char *str, const char *prefix)
ML_FN_READ_PROPERTY ML_FN_HOT;

bool
ml_string_has_suffix (const char *str, const char *suffix)
ML_FN_READ_PROPERTY ML_FN_HOT;

bool
ml_string_matches (const char *str, const char *pattern, bool case_sensitive);

char *
ml_string_trim (char *str)
ML_FN_HOT;

char **
ml_string_split (const char *str, const char *delim)
ML_FN_HOT;

char *
ml_string_replace_all (const char *src, char *dest, char needle, char replacement)
ML_FN_HOT;

// this function can be used with ml_grow_array_sort (with qsort too)
int
ml_string_compare_func (const char **s1, const char **s2);


/* String formatting functions */

void
ml_string_format_size_s (uint64_t value, const char *unit, bool si_units, char *buf, size_t buflen)
ML_FN_HOT;

void
ml_string_format_time_s (uint64_t seconds, char *buf, size_t buflen)
ML_FN_HOT;

char *
ml_strdup_printf (const char *format, ...)
ML_FN_PRINTF(1,2) ML_FN_HOT;

char *
ml_strdup_vprintf (const char *format, va_list va)
ML_FN_HOT;

int
ml_snprintf_append (char *s, size_t n, const char *format, ...)
ML_FN_PRINTF(3,4) ML_FN_HOT;


ML_HEADER_END
#endif /* ML_HEADER__UTIL_STRING_H__INCLUDED */
