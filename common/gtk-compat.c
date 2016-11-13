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


#if ! GTK_CHECK_VERSION(3,4,0)

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

#endif /* ! GTK_CHECK_VERSION(3,4,0) */


#if ! GTK_CHECK_VERSION(3,0,0)

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

#endif /* ! GTK_CHECK_VERSION(3,0,0) */


#if ! GTK_CHECK_VERSION(3,10,0)

GtkWidget *
gtk_button_new_from_icon_name (const gchar *icon_name, GtkIconSize size)
{
	GtkWidget *ret = gtk_button_new();
	GtkWidget *image = gtk_image_new_from_icon_name(icon_name, size);
	gtk_button_set_image(GTK_BUTTON(ret), image);
	return ret;
}

#endif /*  ! GTK_CHECK_VERSION(3,10,0) */


#if ! GTK_CHECK_VERSION(3,16,0)

G_GNUC_INTERNAL void
gtk_label_set_xalign (GtkLabel *label, gfloat xalign) {
	gfloat yalign;
	gtk_misc_get_alignment (GTK_MISC(label), NULL, &yalign);
	gtk_misc_set_alignment (GTK_MISC(label), xalign, yalign);
}

G_GNUC_INTERNAL void
gtk_label_set_yalign (GtkLabel *label, gfloat yalign) {
	gfloat xalign;
	gtk_misc_get_alignment (GTK_MISC(label), &xalign, NULL);
	gtk_misc_set_alignment (GTK_MISC(label), xalign, yalign);
}

#endif /* ! GTK_CHECK_VERSION(3,16,0) */


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

