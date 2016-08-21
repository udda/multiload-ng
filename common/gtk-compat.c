/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *               2002 The Free Software Foundation
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


#include <config.h>

#include "gtk-compat.h"


// Compatibility functions for GTK2
#if GTK_API == 2


gboolean
gdk_rgba_parse (GdkRGBA* color, const gchar* gspec) {
	GdkColor c;
	gboolean ret;
	ret = gdk_color_parse(gspec, &c);
	if (ret)
		gdk_color_to_rgba(&c, 0xffff, color);

	return ret;
}

void
gtk_color_chooser_get_rgba (GtkColorButton *chooser, GdkRGBA *color)
{
	GdkColor c;
	gtk_color_button_get_color(chooser, &c);
	gdk_color_to_rgba(&c, gtk_color_button_get_alpha(chooser), color);
}

void
gtk_color_chooser_set_rgba (GtkColorButton *chooser, GdkRGBA *color)
{
	GdkColor c;
	guint16 alpha;
	gdk_rgba_to_color(color, &c, &alpha);

	gtk_color_button_set_color(chooser, &c);
	gtk_color_button_set_alpha(chooser, alpha);
}

G_GNUC_INTERNAL void
gtk_color_chooser_set_use_alpha (GtkColorButton *chooser, gboolean use_alpha)
{
	gtk_color_button_set_use_alpha(chooser, use_alpha);
}


GtkWidget *
gtk_color_button_new_with_rgba (const GdkRGBA *rgba)
{
	GdkColor c;
	guint16 alpha;
	GtkWidget *ret;
	gdk_rgba_to_color(rgba, &c, &alpha);

	ret = gtk_color_button_new_with_color(&c);
	gtk_color_button_set_alpha (GTK_COLOR_BUTTON(ret), alpha);
	return ret;
}


GtkWidget *
gtk_box_new (GtkOrientation o, guint spacing)
{
	if (o == GTK_ORIENTATION_HORIZONTAL)
		return gtk_hbox_new(FALSE, spacing);
	else // if (o == GTK_ORIENTATION_VERTICAL)
		return gtk_vbox_new(FALSE, spacing);
}

GtkWidget *
gtk_separator_new (GtkOrientation o)
{
	if (o == GTK_ORIENTATION_HORIZONTAL)
		return gtk_hseparator_new();
	else // if (o == GTK_ORIENTATION_VERTICAL)
		return gtk_vseparator_new();
}


GtkWidget *
gtk_button_new_from_icon_name (const gchar *icon_name, GtkIconSize size)
{
	GtkWidget *ret = gtk_button_new();
	GtkWidget *image = gtk_image_new_from_icon_name(icon_name, size);
	gtk_button_set_image(GTK_BUTTON(ret), image);
	return ret;
}

#else  /* GTK_API == 2 */

#endif /* GTK_API == 2 */


void
gdk_color_to_rgba(const GdkColor *color, guint16 alpha, GdkRGBA *rgba)
{
	rgba->red = color->red / 65535.0;
	rgba->green = color->green / 65535.0;
	rgba->blue = color->blue / 65535.0;
	rgba->alpha = alpha / 65535.0;
}

void
gdk_rgba_to_color(const GdkRGBA *rgba, GdkColor *color, guint16 *alpha)
{
	color->red = 65535 * rgba->red;
	color->green = 65535 * rgba->green;
	color->blue = 65535 * rgba->blue;
	if (alpha != NULL)
		*alpha = 65535 * rgba->alpha;
}

