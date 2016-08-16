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


#if GTK_API == 2


GtkWidget *
gtk_box_new (GtkOrientation o, guint spacing)
{
	if (o == GTK_ORIENTATION_HORIZONTAL)
		return gtk_hbox_new(FALSE, spacing);
	else // if (o == GTK_ORIENTATION_VERTICAL)
		return gtk_vbox_new(FALSE, spacing);
}

GtkWidget*
gtk_separator_new (GtkOrientation o)
{
	if (o == GTK_ORIENTATION_HORIZONTAL)
		return gtk_hseparator_new();
	else // if (o == GTK_ORIENTATION_VERTICAL)
		return gtk_vseparator_new();
}

#endif /* GTK_API == 2 */
