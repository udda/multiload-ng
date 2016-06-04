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


#ifndef __MULTILOAD_COLORS_H__
#define __MULTILOAD_COLORS_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include "multiload.h"


G_BEGIN_DECLS

typedef enum {
	EXTRA_COLOR_BORDER = 0,
	EXTRA_COLOR_BACKGROUND_TOP,
	EXTRA_COLOR_BACKGROUND_BOTTOM,

	EXTRA_COLORS
} MultiloadExtraColor;

G_GNUC_INTERNAL guint
multiload_colors_get_extra_index(guint i, MultiloadExtraColor col);

G_GNUC_INTERNAL void
multiload_colors_stringify(MultiloadPlugin *ma, guint i, char *list);
G_GNUC_INTERNAL void
multiload_colors_default(MultiloadPlugin *ma, guint i);
G_GNUC_INTERNAL gboolean
multiload_colors_unstringify(MultiloadPlugin *ma, guint i, const char *list);

G_GNUC_INTERNAL gboolean
multiload_colors_from_file(const gchar *filename, MultiloadPlugin *ma, GtkWindow *parent);
G_GNUC_INTERNAL gboolean
multiload_colors_to_file(const gchar *filename, MultiloadPlugin *ma, GtkWindow *parent);

G_END_DECLS

#endif /* __MULTILOAD_COLORS_H__ */
