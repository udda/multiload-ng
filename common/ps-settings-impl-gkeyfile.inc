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
 * Include this file in any plugin.c to make it use GKeyFile (standard INI-like files).
 *
 * PREREQUISITES
 * -------------
 * Before including, #define MULTILOAD_CONFIG_BASENAME as the name (without path) of config file.
*/

#include <glib.h>

#define G_KEY_GROUP_NAME "Multiload"

#ifndef MULTILOAD_CONFIG_BASENAME
#error "MULTILOAD_CONFIG_BASENAME must be defined before including this file"
#endif


static gchar*
build_config_filename() {
	return g_build_filename(MULTILOAD_CONFIG_PATH, MULTILOAD_CONFIG_BASENAME, NULL);
}


gpointer
multiload_ps_settings_open_for_read(MultiloadPlugin *ma)
{
	GKeyFile *gkf;
	GError *err = NULL;
	gboolean res;
	gchar* fname = build_config_filename();

	gkf = g_key_file_new();
	if (g_file_test(fname, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
		res = g_key_file_load_from_file(gkf, fname, G_KEY_FILE_NONE, &err);
		if (!res) {
			g_warning("multiload_ps_settings_open_for_read: %s", err->message);
			g_key_file_free(gkf);
			gkf = NULL;
		}
	}

	g_clear_error(&err);
	return gkf;
}
gpointer
multiload_ps_settings_open_for_save(MultiloadPlugin *ma)
{
	return g_key_file_new();
}

gboolean
multiload_ps_settings_save(gpointer settings)
{
	GError *err = NULL;
	gboolean res;
	gchar* fname = build_config_filename();

	// avoid to use g_key_file_save_to_file because it's available only for glib>=2.40
	gchar* settings_data = g_key_file_to_data ((GKeyFile*)settings, NULL, NULL);
	res = g_file_set_contents (fname, settings_data, -1, &err);

	if (!res)
		g_warning("multiload_ps_settings_save: %s", err->message);

	g_free(fname);
	g_free(settings_data);
	g_clear_error(&err);
	return res;
}

void
multiload_ps_settings_close(gpointer settings)
{
	g_key_file_free((GKeyFile*)settings);
}

gboolean
multiload_ps_settings_get_int(gpointer settings, const gchar *key, int *destination)
{
	GError *err = NULL;
	gboolean ret;
	int temp;

	temp = g_key_file_get_integer ((GKeyFile*)settings, G_KEY_GROUP_NAME, key, &err);
	if (G_LIKELY(err == NULL || err->code == 0)) {
		*destination = temp;
		ret = TRUE;
	} else {
		ret = FALSE;
	}

	g_clear_error(&err);
	return ret;
}
gboolean
multiload_ps_settings_get_boolean(gpointer settings, const gchar *key, gboolean *destination)
{
	GError *err = NULL;
	gboolean ret;
	gboolean temp;

	temp = g_key_file_get_boolean ((GKeyFile*)settings, G_KEY_GROUP_NAME, key, &err);
	if (G_LIKELY(err == NULL || err->code == 0)) {
		*destination = temp;
		ret = TRUE;
	} else {
		ret = FALSE;
	}

	g_clear_error(&err);
	return ret;
}
gboolean
multiload_ps_settings_get_string(gpointer settings, const gchar *key, gchar *destination, size_t maxlen)
{
	GError *err = NULL;
	gboolean ret;
	gchar *temp;

	temp = g_key_file_get_string ((GKeyFile*)settings, G_KEY_GROUP_NAME, key, &err);
	if (G_LIKELY((err == NULL || err->code == 0) && temp != NULL)) {
		strncpy(destination, temp, maxlen);
		ret = TRUE;
	} else {
		ret = FALSE;
	}

	g_clear_error(&err);
	g_free(temp);
	return ret;
}

void
multiload_ps_settings_set_int(gpointer settings, const gchar *key, int value)
{
	g_key_file_set_integer ((GKeyFile*)settings, G_KEY_GROUP_NAME, key, value);
}
void
multiload_ps_settings_set_boolean(gpointer settings, const gchar *key, gboolean value)
{
	g_key_file_set_boolean ((GKeyFile*)settings, G_KEY_GROUP_NAME, key, value);
}
void
multiload_ps_settings_set_string(gpointer settings, const gchar *key, const gchar *value)
{
	g_key_file_set_string ((GKeyFile*)settings, G_KEY_GROUP_NAME, key, value);
}

void
multiload_ps_preferences_closed_cb(MultiloadPlugin *ma)
{
}
