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

// Compatibility functions for GTK2
G_BEGIN_DECLS


#if GTK_API == 2

G_GNUC_INTERNAL GtkWidget *
gtk_box_new (GtkOrientation o, guint spacing);

G_GNUC_INTERNAL GtkWidget*
gtk_separator_new (GtkOrientation o);

#else  /* GTK_API == 2 */

#endif /* GTK_API == 2 */


G_END_DECLS

#endif /* __GTK_COMPAT_H__ */
