/*  $Id$
 *
 *  Copyright (C) 2012 nandhp <nandhp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __XFCE4_MULTILOAD_PLUGIN_H__
#define __XFCE4_MULTILOAD_PLUGIN_H__

#include "multiload.h"
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>

G_BEGIN_DECLS

typedef struct _MultiloadXfcePlugin MultiloadXfcePlugin;

struct _MultiloadXfcePlugin
{
	MultiloadPlugin ma;

	/* panel widgets */
	XfcePanelPlugin *plugin;
	GtkWidget       *ebox;
};

G_END_DECLS

#endif /* __XFCE4_MULTILOAD_PLUGIN_H__ */
