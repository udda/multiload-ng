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
ml_strv_contains (char **strv, const char *needle)
{
	if_unlikely (strv == NULL || needle == NULL)
		return false;

	ml_strv_for (strv, i) {
		if (!strcmp (strv[i], needle))
			return true;
	}

	return false;
}

bool
ml_strv_contains_prefix (char **strv, const char *needle)
{
	if_unlikely (strv == NULL || needle == NULL)
		return false;

	ml_strv_for (strv, i) {
		if (!ml_string_has_prefix (strv[i], needle))
			return true;
	}

	return false;
}

bool
ml_strv_contains_suffix (char **strv, const char *needle)
{
	if_unlikely (strv == NULL || needle == NULL)
		return false;

	ml_strv_for (strv, i) {
		if (!ml_string_has_suffix (strv[i], needle))
			return true;
	}

	return false;
}

char *
ml_strv_join (char **strv, const char *separator)
{
	if_unlikely (strv == NULL)
		return NULL;

	MlGrowBuffer *gbuf = ml_grow_buffer_new (32);

	if (!ml_strv_join_to_grow_buffer (strv, separator, gbuf)) {
		ml_grow_buffer_destroy (gbuf, true);
		return NULL;
	}

	return (char*)ml_grow_buffer_destroy (gbuf, false);
}

bool
ml_strv_join_to_grow_buffer (char **strv, const char *separator, MlGrowBuffer *gbuf)
{
	if_unlikely (strv == NULL || gbuf == NULL)
		return false;

	ml_strv_for (strv, i) {
		ml_grow_buffer_append_string (gbuf, strv[i]);

		if (strv[i+1] != NULL && strv[i+1][0] != '\0' && separator != NULL)
			ml_grow_buffer_append_string (gbuf, (char*)separator);
	}

	return true;
}

size_t
ml_strv_length (char **strv)
{
	if_unlikely (strv == NULL)
		return 0;

	int len = 0;

	while (strv[len] != NULL)
		len++;

	return len;
}

size_t
ml_strv_sizeof (char **strv)
{
	if_unlikely (strv == NULL)
		return 0;

	size_t size = sizeof (char*); // terminating NULL element

	ml_strv_for (strv, i)
		size += sizeof (char*) + ml_string_sizeof(strv[i]);

	return size;
}

void
ml_strv_free (char **strv)
{
	if_unlikely (strv == NULL)
		return;

	ml_strv_for (strv, i)
		free (strv[i]);

	free (strv);
}

char *
ml_strdup (const char *str)
{
	if (str == NULL)
		return NULL;

	return strdup(str);
}

size_t
ml_string_sizeof (char *str)
{
	if_unlikely (str == NULL)
		return 0;

	return strlen (str) + 1; // +1 for terminating NULL character
}

bool
ml_string_equals (const char *str1, const char *str2, bool case_sensitive)
{
	if (str1 == NULL || str2 == NULL)
		return false;

	if (case_sensitive)
		return strcmp (str1, str2) == 0;
	else
		return strcasecmp (str1, str2) == 0;
}

bool
ml_string_has_prefix (const char *str, const char *prefix)
{
	if (str == NULL || prefix == NULL)
		return false;

	size_t prefix_len = strlen (prefix);
	size_t str_len = strlen (str);

	if (prefix_len == 0 && str_len == 0)
		return true;

	if (prefix_len == 0 || str_len == 0 || prefix_len > str_len)
		return false;

	return strncmp (str, prefix, prefix_len) == 0;
}

bool
ml_string_has_suffix (const char *str, const char *suffix)
{
	if (str == NULL || suffix == NULL)
		return false;

	size_t suffix_len = strlen (suffix);
	size_t str_len = strlen (str);

	if (suffix_len == 0 && str_len == 0)
		return true;

	if (suffix_len == 0 || str_len == 0 || suffix_len > str_len)
		return false;

	return strncmp (str + (str_len - suffix_len), suffix, suffix_len) == 0;
}

bool
ml_string_matches (const char *str, const char *pattern, bool case_sensitive)
{
	regex_t re;

	int r = regcomp (&re, pattern,
		REG_EXTENDED |	// enable extended regular expressions
		REG_NOSUB |		// do not report position of matches
		REG_NEWLINE | 	// no multiline match (each line is a separate string to match)
		(case_sensitive ? 0 : REG_ICASE)	// case insensitive match
	);

	if (r != 0) {
    	char buf_err[200];
		regerror (r, &re, buf_err, sizeof(buf_err));
		ml_error ("Cannot compile pattern '%s': %s", pattern, buf_err);
		return false;
	}

	int ret = regexec (&re, str, 0, NULL, 0);
	regfree (&re);

	if (ret == 0)
		return true;

	return false;
}

char *
ml_string_trim (char *str)
{
	// note: this function modifies string in place

	if (str == NULL)
		return NULL;

	size_t len = strlen (str);

	// remove trailing spaces
	while (len > 0 && ml_isspace (str[len-1]))
		len--;
	str[len] = '\0';

	// remove leading spaces
	size_t i = 0;
	while (i < len && ml_isspace (str[i]))
		i++;
	if (i > 0)
		memmove (str, str + i, len - i + 1);

	// return string itself
	return str;
}

char **
ml_string_split (const char *str, const char *delim)
{
	if (str == NULL)
		return NULL;

	/* NOTE: here delim is the array of possible
	 * delimiter characters, not a delimiter string. */

	// original string is modified by strsep, so we make a copy of it
	char *dup = ml_strdup (str);
	char *pch = dup;

	MlGrowArray *ga = ml_grow_array_new (5, NULL);
	char *token;
	while ((token = strsep (&pch, delim)) != NULL) {
		if (token[0] != '\0')
			ml_grow_array_append (ga, ml_strdup(token));
	}

	// cleanup
	free(dup);

	return (char**)ml_grow_array_destroy (ga, false, true);
}

char *
ml_string_replace_all (const char *src, char *dest, char needle, char replacement)
{
	if_unlikely (src == NULL || dest == NULL)
		return NULL;

	for (int i = 0; src[i] != '\0'; i++) {
		if (src[i] == needle)
			dest[i] = replacement;
		else
			dest[i] = src[i];
	}

	return dest;
}

int
ml_string_compare_func (const char **s1, const char **s2)
{
	if (s1 == NULL || s2 == NULL || *s1 == NULL || *s2 == NULL)
		return 0;

	return strcmp (*s1, *s2);
}

void
ml_string_format_size_s (uint64_t value, const char *unit, bool si_units, char *buf, size_t buflen)
{
	unsigned mul = si_units ? 1000 : 1024;
	static const char* prefix_1000[] = { "", "K",  "M",  "G",  "T"  };
	static const char* prefix_1024[] = { "", "Ki", "Mi", "Gi", "Ti" };

	double v = (double)value;
	int steps = 0;

	while (v > mul) {
		v /= mul;
		steps ++;

		if (steps >= 4)
			break;
	}

	if (steps == 0)
		snprintf (buf, buflen, "%"PRIu64" %s", value, unit);
	else
		snprintf (buf, buflen, "%0.01f %s%s", v, si_units ? prefix_1000[steps] : prefix_1024[steps], unit);
}

void
ml_string_format_time_s (uint64_t seconds, char *buf, size_t buflen)
{
	if_unlikely (buf == NULL)
		return;

	unsigned v;
	buf[0] = '\0';

	// weeks
	if (seconds >= (60 * 60 * 24 * 7)) {
		v = seconds / (60 * 60 * 24 * 7);
		seconds -= v * (60 * 60 * 24 * 7);
		ml_snprintf_append (buf, buflen, "%u %s, ", v, (v==1 ? _("week") : _("weeks")));
	}

	// days
	if (seconds >= (60 * 60 * 24)) {
		v = seconds / (60 * 60 * 24);
		seconds -= v * (60 * 60 * 24);
		ml_snprintf_append (buf, buflen, "%u %s, ", v, (v==1 ? _("day") : _("days")));
	}

	// hours
	if (seconds >= (60 * 60)) {
		v = seconds / (60 * 60);
		seconds -= v * (60 * 60);
		ml_snprintf_append (buf, buflen, "%u %s, ", v, (v==1 ? _("hour") : _("hours")));
	}

	// minutes
	if (seconds >= (60)) {
		v = seconds / (60);
		seconds -= v * (60);
		ml_snprintf_append (buf, buflen, "%u %s, ", v, (v==1 ? _("minute") : _("minutes")));
	}

	// seconds
	if (seconds > 0 || buf[0] == '\0') {
		v = seconds;
		ml_snprintf_append (buf, buflen, "%u %s, ", v, (v==1 ? _("second") : _("seconds")));
	}

	buf [strlen(buf) - 2] = '\0';
}

char *
ml_strdup_printf (const char *format, ...)
{
	if_unlikely (format == NULL)
		return NULL;

	va_list va;
	va_start (va, format);
	char *ret = ml_strdup_vprintf (format, va);
	va_end (va);

	return ret;
}

char *
ml_strdup_vprintf (const char *format, va_list va)
{
	if_unlikely (format == NULL)
		return NULL;

	char buf[ML_PRINTF_STATIC_BUFFER_SIZE]; // see global.h to learn about buffer size

	vsnprintf (buf, sizeof(buf), format, va);
	return strdup (buf);
}

int
ml_snprintf_append (char *s, size_t n, const char *format, ...)
{
	if_unlikely (s == NULL || n == 0)
		return 0;

	size_t len = strlen (s);
	if (len >= n)
		return 0;

	va_list va;
	va_start (va, format);
	int ret = vsnprintf (s+len, n-len, format, va);
	va_end (va);

	return ret;
}
