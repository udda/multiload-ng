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


#ifndef __MULTILOAD_INFO_FILE_H__
#define __MULTILOAD_INFO_FILE_H__

#include <glib.h>
#include <stdio.h>


typedef struct {
	gchar *key;
	gchar type; // one of 's', 'i', 'u', 'd'
	gpointer address;
} InfoFileMappingEntry;


G_GNUC_INTERNAL
FILE*
info_file_required_fopen (const gchar *path, const gchar *mode);

G_GNUC_INTERNAL
gboolean
info_file_exists (const gchar *path);

G_GNUC_INTERNAL
gboolean
info_file_has_contents (const gchar *path, const gchar *contents);

G_GNUC_INTERNAL
gboolean
info_file_read_string_s (const gchar *path, gchar *buf, const size_t bufsize, size_t *length);

G_GNUC_INTERNAL
gboolean
info_file_read_string (const gchar *path, gchar **out, size_t *length);

G_GNUC_INTERNAL
gboolean
info_file_read_int64 (const gchar *path, gint64 *out);

G_GNUC_INTERNAL
gboolean
info_file_read_uint64 (const gchar *path, guint64 *out);

G_GNUC_INTERNAL
gboolean
info_file_read_double (const gchar *path, gdouble *out, gdouble scale);


G_GNUC_INTERNAL
gboolean
info_file_read_key_string_s (const gchar *path, const gchar *key, gchar *buf, size_t bufsize, size_t *length);

G_GNUC_INTERNAL
gboolean
info_file_read_key_int64 (const gchar *path, const gchar *key, gint64 *out);

G_GNUC_INTERNAL
gboolean
info_file_read_key_uint64 (const gchar *path, const gchar *key, guint64 *out);

G_GNUC_INTERNAL
gboolean
info_file_read_key_double (const gchar *path, const gchar *key, gdouble *out, gdouble scale);


G_GNUC_INTERNAL
gint
info_file_read_keys (const gchar *path, const InfoFileMappingEntry *entries, gint count);

G_GNUC_INTERNAL
guint
info_file_count_key_values (const gchar *path, const gchar *key);

#endif /* __MULTILOAD_INFO_FILE_H__ */
