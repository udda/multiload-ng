/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
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


#ifndef __UTIL_H__
#define __UTIL_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

G_GNUC_INTERNAL guint64
calculate_speed(guint64 delta, guint period_ms);
G_GNUC_INTERNAL char*
str_replace(const char *string, const char *needle, const char *replacement);


G_GNUC_INTERNAL gboolean
file_check_contents(FILE *f, const gchar *string);
G_GNUC_INTERNAL gint64
read_int_from_file(const gchar *path);

G_GNUC_INTERNAL gchar*
format_size_for_display (guint size, gboolean iec_units);
G_GNUC_INTERNAL gchar*
format_rate_for_display (guint rate, gboolean iec_units);
G_GNUC_INTERNAL gchar*
format_percent(guint64 value, guint64 total, guint ndigits);
G_GNUC_INTERNAL gchar*
format_time_duration(gdouble seconds);

G_GNUC_INTERNAL void
show_modal_info_dialog(GtkWindow *parent, GtkMessageType type, const gchar *message);

G_GNUC_INTERNAL gchar*
get_system_monitor_executable();
G_GNUC_INTERNAL void
xdg_open_url();

G_GNUC_INTERNAL FILE*
cached_fopen_r(gchar* path, gboolean reopen);

G_GNUC_INTERNAL GdkPixbuf*
cairo_surface_to_gdk_pixbuf(cairo_surface_t *surface, guint width, guint height);

G_END_DECLS

#endif /* __UTIL_H__ */
