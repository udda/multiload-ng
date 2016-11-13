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


#ifndef __GTK_COMPAT_H__
#define __GTK_COMPAT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS


#if ! GTK_CHECK_VERSION(3,0,0)

typedef struct {
	gdouble red;
	gdouble green;
	gdouble blue;
	gdouble alpha;
} GdkRGBA;
#define GTK_COLOR_CHOOSER GTK_COLOR_BUTTON

#endif /* ! GTK_CHECK_VERSION(3,0,0) */


#if ! GTK_CHECK_VERSION(3,4,0)

G_GNUC_INTERNAL gboolean
gdk_rgba_parse (GdkRGBA* color, const gchar* gspec);

G_GNUC_INTERNAL void
gtk_color_chooser_get_rgba (GtkColorButton *chooser, GdkRGBA *color);

G_GNUC_INTERNAL void
gtk_color_chooser_set_rgba (GtkColorButton *chooser, GdkRGBA *color);

G_GNUC_INTERNAL void
gtk_color_chooser_set_use_alpha (GtkColorButton *chooser, gboolean use_alpha);

G_GNUC_INTERNAL GtkWidget *
gtk_color_button_new_with_rgba (const GdkRGBA *rgba);

#endif /* ! GTK_CHECK_VERSION(3,4,0) */


#if ! GTK_CHECK_VERSION(3,0,0)

G_GNUC_INTERNAL GtkWidget *
gtk_box_new (GtkOrientation o, guint spacing);

G_GNUC_INTERNAL GtkWidget *
gtk_separator_new (GtkOrientation o);

#endif /* ! GTK_CHECK_VERSION(3,0,0) */


#if ! GTK_CHECK_VERSION(3,10,0)

G_GNUC_INTERNAL GtkWidget *
gtk_button_new_from_icon_name (const gchar *icon_name, GtkIconSize size);

#endif /* ! GTK_CHECK_VERSION(3,10,0) */


//gtk+>3.4
#ifndef GDK_EVENT_PROPAGATE
#define GDK_EVENT_PROPAGATE (FALSE)
#endif

#ifndef GDK_EVENT_STOP
#define GDK_EVENT_STOP (TRUE)
#endif


void
gdk_color_to_rgba(const GdkColor *color, guint16 alpha, GdkRGBA *rgba);

void
gdk_rgba_to_color(const GdkRGBA *rgba, GdkColor *color, guint16 *alpha);


G_END_DECLS

#endif /* __GTK_COMPAT_H__ */
