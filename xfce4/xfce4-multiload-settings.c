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

#include <config.h>
#include <string.h>

#include "multiload.h"
#include "multiload-config.h"
#include "multiload-colors.h"
#include "xfce4-multiload-plugin.h"
#include "xfce4-multiload-dialogs.h"

void
multiload_save (XfcePanelPlugin *plugin, MultiloadPlugin *ma)
{
	XfceRc *rc;
	gchar *file;
	guint i;

	/* get the config file location */
	file = xfce_panel_plugin_save_location (plugin, TRUE);

	if (G_UNLIKELY (file == NULL)) {
		DBG ("Failed to open config file");
		return;
	}

	/* open the config file, read/write */
	rc = xfce_rc_simple_open (file, FALSE);
	g_free (file);

	if (G_LIKELY (rc != NULL)) {
		/* save the settings */
		DBG(".");

		/* Write common config */
		xfce_rc_write_int_entry (rc, "speed", ma->speed);
		xfce_rc_write_int_entry (rc, "size", ma->size);
		xfce_rc_write_int_entry (rc, "padding", ma->padding);
		xfce_rc_write_int_entry (rc, "spacing", ma->spacing);
		xfce_rc_write_int_entry (rc, "orientation", ma->orientation_policy);
		xfce_rc_write_int_entry (rc, "fill-between", ma->fill_between);
		xfce_rc_write_int_entry (rc, "tooltip-details", ma->tooltip_details);
		xfce_rc_write_int_entry (rc, "dblclick-policy", ma->dblclick_policy);
		xfce_rc_write_entry (rc, "dblclick-cmdline", ma->dblclick_cmdline);

		for ( i = 0; i < GRAPH_MAX; i++ ) {
			char *key, list[10*MAX_COLORS];

			/* Visibility */
			key = g_strdup_printf("%s_visible", graph_types[i].name);
			xfce_rc_write_bool_entry (rc, key, ma->graph_config[i].visible);
			g_free (key);

			/* Border width */
			key = g_strdup_printf("%s_border-width", graph_types[i].name);
			xfce_rc_write_int_entry (rc, key, ma->graph_config[i].border_width);
			g_free (key);

			/* Save colors */
			multiload_colors_stringify (ma, i, list);
			key = g_strdup_printf("%s_colors", graph_types[i].name);
			xfce_rc_write_entry (rc, key, list);
			g_free (key);
		}

		/* close the rc file */
		xfce_rc_close (rc);
	}
}

void
multiload_read (XfcePanelPlugin *plugin,
				MultiloadPlugin *ma)
{
	XfceRc *rc;
	gchar *file;
	guint i;
	gchar *tmp_str;

	multiload_defaults(ma);

	/* get the plugin config file location */
	file = xfce_panel_plugin_lookup_rc_file (plugin);

	if (G_LIKELY (file != NULL)) {
		/* open the config file, readonly */
		rc = xfce_rc_simple_open (file, TRUE);

		/* cleanup */
		g_free (file);

		if (G_LIKELY (rc != NULL)) {
			/* Read speed and size */
			ma->speed = xfce_rc_read_int_entry(rc, "speed", DEFAULT_SPEED);
			ma->size = xfce_rc_read_int_entry(rc, "size", DEFAULT_SIZE);
			ma->padding = xfce_rc_read_int_entry(rc, "padding", DEFAULT_PADDING);
			ma->spacing = xfce_rc_read_int_entry(rc, "spacing", DEFAULT_PADDING);
			ma->orientation_policy = xfce_rc_read_int_entry (rc, "orientation", DEFAULT_ORIENTATION);
			ma->fill_between = xfce_rc_read_bool_entry (rc, "fill-between", DEFAULT_FILL_BETWEEN);
			ma->tooltip_details = xfce_rc_read_bool_entry (rc, "tooltip-details", DEFAULT_TOOLTIP_DETAILS);
			ma->dblclick_policy = xfce_rc_read_int_entry (rc, "dblclick-policy", DEFAULT_DBLCLICK_POLICY);
			tmp_str = xfce_rc_read_entry (rc, "dblclick-cmdline", NULL);
			if (tmp_str != NULL)
				strncpy(ma->dblclick_cmdline, tmp_str, sizeof(ma->dblclick_cmdline)/sizeof(gchar));

			/* Read visibility and colors for each graph */
			for ( i = 0; i < GRAPH_MAX; i++ ) {
				char *key;
				const char *list;

				/* Visibility */
				key = g_strdup_printf("%s_visible", graph_types[i].name);
				ma->graph_config[i].visible = xfce_rc_read_bool_entry(rc, key, (i==0 ? TRUE:FALSE));
				g_free (key);

				/* Border width */
				key = g_strdup_printf("%s_border-width", graph_types[i].name);
				ma->graph_config[i].border_width = xfce_rc_read_int_entry(rc, key, DEFAULT_BORDER_WIDTH);
				g_free (key);

				/* Colors - Try to load from xfconf */
				key = g_strdup_printf("%s_colors", graph_types[i].name);
				list = xfce_rc_read_entry (rc, key, NULL);
				g_free (key);
				multiload_colors_unstringify(ma, i, list);
			}

			/* cleanup */
			xfce_rc_close (rc);

			multiload_sanitize(ma);

			/* leave the function, everything went well */
			return;
		}
	}
}
