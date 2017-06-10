/*
 * Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of Multiload-ng.
 *
 * Multiload-ng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Multiload-ng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <multiload-server.h>


// <editor-fold> Simple JSON generation
static char *
_generate_simple_json_string (const char *str)
{
	if (str == NULL || str[0] == '\0') {
		char buf[30];
		snprintf (buf, sizeof(buf), "{ \"value\": \"\" }");
		return strdup (buf);
	}

	size_t len = 20 + 2 * strlen(str); // enough room for escaping every character
	char *ret = malloc (len);

	size_t pos = snprintf (ret, len, "{ \"value\": \"");

	// escape value
	size_t i;
	for (i=0; i < strlen(str); i++) {
		if (str[i] == '\\' || str[i] == '"')
			ret[pos++] = '\\';

		ret[pos++] = str[i];
	}

	snprintf (ret+pos, len-pos, "\" }");

	return ret;
}

static char *
_generate_simple_json_bool (bool b)
{
	char buf[30];
	snprintf (buf, sizeof(buf), "{ \"value\": %s }", b?"true":"false");

	return strdup (buf);
}

static char *
_generate_simple_json_int (int d)
{
	char buf[30];
	snprintf (buf, sizeof(buf), "{ \"value\": %d }", d);

	return strdup (buf);
}
// </editor-fold>


char *
multiload_server_command_status (MultiloadServer *mlsrv, ML_UNUSED struct MHD_Connection *connection)
{
	// handles the case of server restarted with client window still open
	if (multiload_server_is_just_started (mlsrv)) {
		multiload_server_clear_just_started (mlsrv);
		return _generate_simple_json_string ("reload");
	}

	// handles the case of element changes
	if (multiload_server_is_dirty_data (mlsrv))
		return _generate_simple_json_string ("dirty_data");


	return _generate_simple_json_string ("ready");
}

char *
multiload_server_command_has_file (MultiloadServer *mlsrv, ML_UNUSED struct MHD_Connection *connection)
{
	return _generate_simple_json_bool (multiload_server_get_filename (mlsrv) != NULL);
}

char *
multiload_server_command_library_version (ML_UNUSED MultiloadServer *mlsrv, ML_UNUSED struct MHD_Connection *connection)
{
	return _generate_simple_json_string (multiload_version());
}

char *
multiload_server_command_data (MultiloadServer *mlsrv, ML_UNUSED struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	char *ret = multiload_to_json (ml);
	if (ret != NULL)
		multiload_server_clear_dirty_data (mlsrv);

	return ret;
}

char *
multiload_server_command_graph_types (ML_UNUSED MultiloadServer *mlsrv, ML_UNUSED struct MHD_Connection *connection)
{
	return multiload_list_graph_types_json ();
}

char *
multiload_server_command_index_at_coords (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int x = multiload_server_connection_get_parameter_int (connection, "x");
	int y = multiload_server_connection_get_parameter_int (connection, "y");

	int index = multiload_get_element_index_at_coords (ml, x, y);

	char buf[60];
	snprintf (buf, sizeof(buf),
		"{ \"index\": %d, \"is_graph\": %s }",
		index,
		multiload_element_is_graph (ml, index) ? "true" : "false"
	);

	return strdup (buf);
}

char *
multiload_server_command_create (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	const char *type = multiload_server_connection_get_parameter_string (connection, "type");
	int size = multiload_server_connection_get_parameter_int (connection, "size");
	int position = multiload_server_connection_get_parameter_int (connection, "position");

	const char *graph_type = multiload_server_connection_get_parameter_string (connection, "graph-type");
	int border = multiload_server_connection_get_parameter_int (connection, "border");
	int interval = multiload_server_connection_get_parameter_int (connection, "interval");

	int new_index;
	if (!strcmp (type, "graph"))
		new_index = multiload_add_graph (ml, graph_type, size, border, interval, position);
	else if (!strcmp (type, "separator"))
		new_index = multiload_add_separator (ml, size, position);
	else
		return NULL;

	multiload_server_set_dirty_data (mlsrv);

	return _generate_simple_json_int (new_index);
}

char *
multiload_server_command_delete (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int index = multiload_server_connection_get_parameter_int (connection, "index");

	multiload_server_set_dirty_data (mlsrv);

	return _generate_simple_json_bool (multiload_element_remove (ml, index));
}

char *
multiload_server_command_resume (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int index = multiload_server_connection_get_parameter_int (connection, "index");

	return _generate_simple_json_bool (multiload_graph_start (ml, index));
}

char *
multiload_server_command_pause (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int index = multiload_server_connection_get_parameter_int (connection, "index");

	return _generate_simple_json_bool (multiload_graph_stop (ml, index));
}

char *
multiload_server_command_move (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int from = multiload_server_connection_get_parameter_int (connection, "from");
	int to = multiload_server_connection_get_parameter_int (connection, "to");

	multiload_server_set_dirty_data (mlsrv);

	return _generate_simple_json_bool (multiload_element_move_to (ml, from, to));
}

char *
multiload_server_command_caption (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int index = multiload_server_connection_get_parameter_int (connection, "index");

	return multiload_graph_get_caption_json (ml, index);
}

char *
multiload_server_command_config_entries (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int index = multiload_server_connection_get_parameter_int (connection, "index");

	return multiload_graph_list_config_entries_json (ml, index);
}

char *
multiload_server_command_get_config (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int index = multiload_server_connection_get_parameter_int (connection, "index");
	const char *key = multiload_server_connection_get_parameter_string (connection, "key");

	return _generate_simple_json_string (multiload_graph_get_config (ml, index, key));
}

char *
multiload_server_command_set_config (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int index = multiload_server_connection_get_parameter_int (connection, "index");
	const char *key = multiload_server_connection_get_parameter_string (connection, "key");
	const char *value = multiload_server_connection_get_parameter_string (connection, "value");

	multiload_server_set_dirty_data (mlsrv);

	return _generate_simple_json_bool (multiload_graph_set_config (ml, index, key, value));
}

char *
multiload_server_command_localization (ML_UNUSED MultiloadServer *mlsrv, ML_UNUSED struct MHD_Connection *connection)
{
	// NOTE: avoid those characters in localized strings:   \b \f \n \r \t \\ \"

	char buf[10240];
	size_t pos = 0;

	pos += snprintf (buf, sizeof(buf), "{\n");

	// NOTE: translation elements have to be explicitly listed in order to be collected by gettext

	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Settings",                                                                  _("Settings"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Element type",                                                              _("Element type"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Graph",                                                                     _("Graph"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Separator",                                                                 _("Separator"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "separator",                                                                 _("separator"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Size",                                                                      _("Size"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Border size",                                                               _("Border size"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Ceiling",                                                                   _("Ceiling"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Update interval (milliseconds)",                                            _("Update interval (milliseconds)"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Graph configuration",                                                       _("Graph configuration"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Graph type",                                                                _("Graph type"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Please pick one",                                                           _("Please pick one"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Please select a graph type",                                                _("Please select a graph type"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "This section will display additional info.",                                _("This section will display additional info."));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Create",                                                                    _("Create"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Create element",                                                            _("Create element"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Cancel",                                                                    _("Cancel"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Ok",                                                                        _("Ok"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Save",                                                                      _("Save"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Reload",                                                                    _("Reload"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Export",                                                                    _("Export"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "About",                                                                     _("About"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Main",                                                                      _("Main"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Elements",                                                                  _("Elements"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\",\n", "Drag and drop to arrange elements, click an element to configure it",       _("Drag and drop to arrange elements, click an element to configure it"));
	pos += snprintf (buf+pos, sizeof(buf)-pos, "\"%s\": \"%s\"\n",  "Unable to contact server",                                                  _("Unable to contact server"));

	snprintf (buf+pos, sizeof(buf)-pos, "}\n");
	return strdup (buf);
}

char *
multiload_server_command_save (MultiloadServer *mlsrv, ML_UNUSED struct MHD_Connection *connection)
{
	return _generate_simple_json_bool (multiload_server_store (mlsrv));
}

char *
multiload_server_command_reload (MultiloadServer *mlsrv, ML_UNUSED struct MHD_Connection *connection)
{
	return _generate_simple_json_bool (multiload_server_reload (mlsrv));
}

char *
multiload_server_command_set_element_size (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int index = multiload_server_connection_get_parameter_int (connection, "index");
	int value = multiload_server_connection_get_parameter_int (connection, "value");

	multiload_server_set_dirty_data (mlsrv);

	return _generate_simple_json_bool (multiload_element_set_size (ml, index, value));
}

char *
multiload_server_command_set_graph_border (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int index = multiload_server_connection_get_parameter_int (connection, "index");
	int value = multiload_server_connection_get_parameter_int (connection, "value");

	multiload_server_set_dirty_data (mlsrv);

	return _generate_simple_json_bool (multiload_graph_set_border_size (ml, index, value));
}

char *
multiload_server_command_set_graph_interval (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int index = multiload_server_connection_get_parameter_int (connection, "index");
	int value = multiload_server_connection_get_parameter_int (connection, "value");

	multiload_server_set_dirty_data (mlsrv);

	return _generate_simple_json_bool (multiload_graph_set_interval (ml, index, value));
}

char *
multiload_server_command_set_graph_ceiling (MultiloadServer *mlsrv, struct MHD_Connection *connection)
{
	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	int index = multiload_server_connection_get_parameter_int (connection, "index");
	int value = multiload_server_connection_get_parameter_int (connection, "value");

	multiload_server_set_dirty_data (mlsrv);

	return _generate_simple_json_bool (multiload_graph_set_ceiling (ml, index, value));
}
