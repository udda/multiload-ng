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


/*
 * Include this file in any plugin.c to make it use GSettings (accessible by dconf).
 *
 * PREREQUISITES
 * -------------
 * In main / plugin constructor / whatever runs before the plugin, a new GSettings must
 * be created and assigned to _settings, e.g: _settings = g_settings_new(...);
*/

#include <glib.h>

static GSettings *_settings = NULL;

gpointer
multiload_ps_settings_open_for_read(MultiloadPlugin *ma)
{
	if (_settings == NULL)
		g_error("_settings == NULL  -  did you remember to create new GSettings* and assign to _settings?");
	return _settings;
}
gpointer
multiload_ps_settings_open_for_save(MultiloadPlugin *ma)
{
	if (_settings == NULL)
		g_error("_settings == NULL  -  did you remember to create new GSettings* and assign to _settings?");
	return _settings;
}

gboolean
multiload_ps_settings_save(gpointer settings)
{
	g_settings_sync();
	return TRUE;
}

void
multiload_ps_settings_close(gpointer settings)
{
}

gboolean
multiload_ps_settings_get_int(gpointer settings, const gchar *key, int *destination)
{
	*destination = g_settings_get_int((GSettings*)settings, key);
	return TRUE;
}
gboolean
multiload_ps_settings_get_boolean(gpointer settings, const gchar *key, gboolean *destination)
{
	*destination = g_settings_get_boolean((GSettings*)settings, key);
	return TRUE;
}
gboolean
multiload_ps_settings_get_string(gpointer settings, const gchar *key, gchar *destination, size_t maxlen)
{
	const gchar* temp = g_settings_get_string((GSettings*)settings, key);
	if (G_LIKELY(temp != NULL))
		strncpy(destination, temp, maxlen);
	return TRUE;
}

void
multiload_ps_settings_set_int(gpointer settings, const gchar *key, int value)
{
	g_settings_set_int((GSettings*)settings, key, value);
}
void
multiload_ps_settings_set_boolean(gpointer settings, const gchar *key, gboolean value)
{
	g_settings_set_boolean((GSettings*)settings, key, value);
}
void
multiload_ps_settings_set_string(gpointer settings, const gchar *key, const gchar *value)
{
	g_settings_set_string((GSettings*)settings, key, value);
}

void
multiload_ps_preferences_closed_cb(MultiloadPlugin *ma)
{
}

