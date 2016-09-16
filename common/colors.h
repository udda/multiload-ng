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


#ifndef __COLORS_H__
#define __COLORS_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>

#include "gtk-compat.h"
#include "multiload.h"


G_BEGIN_DECLS

#define HEX_TO_RGBA(r, g, b, a) {(gdouble)(0x##r)/255.0, (gdouble)(0x##g)/255.0, (gdouble)(0x##b)/255.0, (gdouble)(0x##a)/255.0}
#define HEX_TO_RGBA_1(rgb, a) {(gdouble)((0x##rgb >> 16) & 0xFF)/255.0, (gdouble)((0x##rgb >> 8) & 0xFF)/255.0, (gdouble)(0x##rgb & 0xFF)/255.0, (gdouble)(0x##a)/255.0}

// Format version. Increment this every time color scheme format changes.
#define MULTILOAD_COLOR_SCHEME_VERSION 4

// Header of the color scheme file. This must never change.
#define MULTILOAD_COLOR_SCHEME_HEADER_SIZE 12
typedef struct {
	char magic[MULTILOAD_COLOR_SCHEME_HEADER_SIZE];
	guint32 version;
	guint8 reserved[16];
} MultiloadColorSchemeFileHeader;

// Color scheme contents
typedef struct {
	char name[24];
	char **xpm_data;
	GdkRGBA colors[GRAPH_MAX][MAX_COLORS];
} MultiloadColorScheme;

typedef enum {
	MULTILOAD_COLOR_SCHEME_STATUS_VALID,
	MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT,
	MULTILOAD_COLOR_SCHEME_STATUS_WRONG_VERSION
} MultiloadColorSchemeStatus;

extern const MultiloadColorScheme multiload_builtin_color_schemes[];

typedef enum {
	EXTRA_COLOR_BORDER = 0,
	EXTRA_COLOR_BACKGROUND_TOP,
	EXTRA_COLOR_BACKGROUND_BOTTOM,

	EXTRA_COLORS
} MultiloadExtraColor;

G_GNUC_INTERNAL guint
multiload_colors_get_extra_index(guint i, MultiloadExtraColor col);


G_GNUC_INTERNAL void
multiload_color_scheme_fill (MultiloadColorScheme *scheme, MultiloadPlugin *ma);

G_GNUC_INTERNAL void
multiload_color_scheme_apply (const MultiloadColorScheme *scheme, MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_color_scheme_apply_single (const MultiloadColorScheme *scheme, MultiloadPlugin *ma, guint i);

G_GNUC_INTERNAL gboolean
multiload_color_scheme_to_file(const gchar *filename, MultiloadPlugin *ma);
G_GNUC_INTERNAL MultiloadColorSchemeStatus
multiload_color_scheme_from_file(const gchar *filename, MultiloadPlugin *ma);

G_GNUC_INTERNAL const MultiloadColorScheme*
multiload_color_scheme_find_by_name (const gchar *name);

G_GNUC_INTERNAL gchar *
multiload_colors_to_string(MultiloadPlugin *ma, guint graph_index);
G_GNUC_INTERNAL gboolean
multiload_colors_from_string(MultiloadPlugin *ma, guint graph_index, const char *list);

G_GNUC_INTERNAL void
multiload_colors_default(MultiloadPlugin *ma, guint graph_index);

G_END_DECLS

#endif /* __COLORS_H__ */
