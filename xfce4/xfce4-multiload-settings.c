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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "multiload.h"
#include "xfce4-multiload-plugin.h"
#include "xfce4-multiload-dialogs.h"

void
multiload_save (XfcePanelPlugin *plugin,
				MultiloadPlugin *ma)
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

		/* Write size and speed */
		xfce_rc_write_int_entry (rc, "speed", ma->speed);
		xfce_rc_write_int_entry (rc, "size", ma->size);
		xfce_rc_write_int_entry (rc, "padding", ma->padding);
		xfce_rc_write_int_entry (rc, "spacing", ma->spacing);
		xfce_rc_write_int_entry (rc, "show-frame", ma->show_frame);

		for ( i = 0; i < NGRAPHS; i++ ) { 
			char *key, list[10*MAX_COLORS];

			/* Visibility */
			key = g_strdup_printf("%s_visible", graph_types[i].name);
			xfce_rc_write_bool_entry (rc, key, ma->graph_config[i].visible);
			g_free (key);

			/* Save colors */
			multiload_colorconfig_stringify (ma, i, list);
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
	guint i, found = 0;

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
			ma->show_frame = xfce_rc_read_int_entry (rc, "show-frame", DEFAULT_SHOWFRAME);

			/* Read visibility and colors for each graph */
			for ( i = 0; i < NGRAPHS; i++ ) {
				char *key;
				const char *list;

				/* Visibility */
				key = g_strdup_printf("%s_visible", graph_types[i].name);
				ma->graph_config[i].visible =
						xfce_rc_read_bool_entry(rc, key, i == 0 ? TRUE : FALSE);
				g_free (key);

				/* Colors - Try to load from xfconf */
				key = g_strdup_printf("%s_colors", graph_types[i].name);
				list = xfce_rc_read_entry (rc, key, NULL);
				g_free (key);
				multiload_colorconfig_unstringify(ma, i, list);
			}

			/* cleanup */
			xfce_rc_close (rc);

			/* Ensure at lease one graph is visible */
			for ( i = 0; i < NGRAPHS; i++ ) {
				if ( ma->graph_config[i].visible == TRUE )
					found++;
				if ( found == 0 )
					ma->graph_config[0].visible = TRUE;
			}

			/* leave the function, everything went well */
			return;
		}
	}

	/* something went wrong, apply default values */
	DBG ("Applying default settings");

	ma->speed = DEFAULT_SPEED;
	ma->size = DEFAULT_SIZE;
	ma->padding = DEFAULT_PADDING;
	ma->spacing = DEFAULT_SPACING;
	ma->show_frame = DEFAULT_SHOWFRAME;
	for ( i = 0; i < NGRAPHS; i++ ) { 
		/* Default visibility and colors */
		ma->graph_config[i].visible = i == 0 ? TRUE : FALSE;
		multiload_colorconfig_default(ma, i);
	}
}
