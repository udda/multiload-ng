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

#include <multiload.h>

#include "multiload.h"


struct _Multiload {
	MlMultiloadContainer *container;
	char theme_name[32];

	MultiloadGraphFailureHandler failure_handler;
	void * failure_handler_data;
};

static void
_multiload_graph_fail_fn (MlGraph *g, int fail_code, const char *fail_msg, Multiload *ml)
{
	if_unlikely (ml == NULL || ml->container == NULL)
		return;

	// convert grap to its index, and call error function if defined
	int graph_index = ml_multiload_container_graph_find_index (ml->container, g);
	if_unlikely (graph_index < 0)
		return;

	// call registered failure handler, fallback to ml_error
	if (ml->failure_handler != NULL)
		ml->failure_handler (ml, graph_index, fail_code, fail_msg, ml->failure_handler_data);
	else
		ml_error ("Error in graph element #%d (type: %s)\n[fail code: %d] %s", graph_index, ml_graph_get_name(g), fail_code, fail_msg);
}

Multiload *
multiload_new (int size, char *orientation, int padding)
{
	ml_multiload_shared_init();

	MlOrientation orient = ml_orientation_parse (orientation);
	if (orient == ML_INVALID) {
		ml_error ("Invalid orientation: '%s'", orientation);
		return NULL;
	}

	Multiload *ml = ml_new (Multiload);
	ml->container = ml_multiload_container_new (size, orient, padding);
	if (ml->container == NULL) {
		ml_error ("Internal error");
		multiload_destroy (ml);
		return NULL;
	}
	ml_multiload_container_set_graph_fail_function (ml->container, (MlGraphFailFunc)_multiload_graph_fail_fn, ml);

	return ml;
}

Multiload *
multiload_new_from_json (const char *json_def)
{
	ml_multiload_shared_init();

	if_unlikely (json_def == NULL)
		return NULL;

	const char *parse_end;
	cJSON *obj = cJSON_ParseWithOpts(json_def, &parse_end, 1);
	if (obj == NULL) {
		ml_error ("Malformed JSON data near character %zu", (size_t)(parse_end - json_def));
		return NULL;
	}

	// product
	cJSON *obj_product = ml_cJSON_GetObjectItem (obj, "product");
	if (obj_product == NULL || !ml_string_equals (obj_product->valuestring, "Multiload-ng", false)) {
		cJSON_Delete (obj);
		return NULL;
	}

	Multiload *ml = ml_new (Multiload);

	// theme (optional)
	cJSON *obj_theme = cJSON_GetObjectItem (obj, "theme");
	if (obj_theme != NULL) {
		strncpy (ml->theme_name, obj_theme->valuestring, sizeof(ml->theme_name));
	} else {
		strncpy (ml->theme_name, "custom", sizeof(ml->theme_name));
	}

	// container
	ml->container = ml_multiload_container_new_from_json (ml_cJSON_GetObjectItem (obj, "container"), ml->theme_name);
	if (ml->container == NULL) {
		free (ml);
		cJSON_Delete (obj);
		return NULL;
	}
	ml_multiload_container_set_graph_fail_function (ml->container, (MlGraphFailFunc)_multiload_graph_fail_fn, ml);


	// shared config (optional)
	ml_config_parse_json (ML_SHARED_GET (config), cJSON_GetObjectItem (obj, "shared-config"));


	cJSON_Delete (obj);

	ml_debug ("Loaded from JSON - size: %zu bytes", multiload_sizeof(ml));
	return ml;
}

Multiload *
multiload_new_from_json_file (const char *path)
{
	if (!ml_file_is_readable(path)) {
		ml_error ("File '%s' does not exist or it's not readable", path);
		return NULL;
	}

	size_t size = ml_file_size (path);
	if (size == 0) {
		ml_error ("File '%s' is empty, or its size cannot be calculated", path);
		return NULL;
	}

	char *buf = ml_new_n (char, size+1);
	size_t readlen;

	if (!ml_infofile_read_string_s (path, buf, size+1, &readlen)) {
		ml_error ("Cannot read JSON data from '%s'", path);
		free (buf);
		return NULL;
	}

	ml_debug ("Loaded %zu byte of JSON data from '%s' (trimmed: %zu byte)", size, path, readlen);
	Multiload *ml = multiload_new_from_json (buf);
	free (buf);

	return ml;
}

void
multiload_destroy (Multiload *ml)
{
	if_unlikely (ml == NULL)
		return;

	ml_multiload_container_destroy (ml->container);
	free (ml);
}

size_t
multiload_sizeof (Multiload *ml)
{
	if_unlikely (ml == NULL)
		return 0;

	size_t size = sizeof (Multiload);
	size += ml_multiload_container_sizeof (ml->container);

	return size;
}

int
multiload_get_size (Multiload *ml)
{
	if_unlikely (ml == NULL)
		return -1;

	return ml_multiload_container_get_size (ml->container);
}

bool
multiload_set_size (Multiload *ml, int newsize)
{
	if_unlikely (ml == NULL)
		return false;

	return ml_multiload_container_set_size (ml->container, newsize);
}

int
multiload_get_length (Multiload *ml)
{
	if_unlikely (ml == NULL)
		return -1;

	return ml_multiload_container_get_length (ml->container);
}

int
multiload_get_padding (Multiload *ml)
{
	if_unlikely (ml == NULL)
		return -1;

	return ml_multiload_container_get_padding (ml->container);
}

bool
multiload_set_padding (Multiload *ml, int newpadding)
{
	if_unlikely (ml == NULL)
		return false;

	return ml_multiload_container_set_padding (ml->container, newpadding);
}

const char *
multiload_get_orientation (Multiload *ml)
{
	if_unlikely (ml == NULL)
		return NULL;

	MlOrientation o = ml_multiload_container_get_orientation (ml->container);
	if_unlikely (o == ML_INVALID)
		return NULL;

	return ml_orientation_to_string (o);
}

bool
multiload_set_orientation (Multiload *ml, char *orientation)
{
	if_unlikely (ml == NULL)
		return false;

	MlOrientation orient = ml_orientation_parse (orientation);
	if (orient == ML_INVALID) {
		ml_error ("Invalid orientation: '%s'", orientation);
		return false;
	}

	return ml_multiload_container_set_orientation (ml->container, orient);
}

int
multiload_add_graph (Multiload *ml, const char *graph_type, int size, int border_size, int interval_ms, int index)
{
	if_unlikely (ml == NULL)
		return -1;

	MlGraphType type = ml_graph_type_parse (graph_type);
	if (type == ML_INVALID) {
		ml_warning ("Unknown graph type '%s'", graph_type);
		return -1;
	}

	const MlGraphTypeInterface *iface = ml_graph_type_interface_get (type);
	if_unlikely (iface == NULL) {
		ml_bug ("Cannot retrieve interface for type '%s' => %d", graph_type, type);
		return -1;
	}

	MlGraphStyle * style = ml_graph_theme_generate_style (ml->theme_name, iface);
	if (style == NULL) {
		ml_warning ("Cannot generate graph style for '%s'", graph_type);
		return -1;
	}

	return ml_multiload_container_add_graph (ml->container, type, size, border_size, style, interval_ms, index);
}

int
multiload_add_separator (Multiload *ml, int size, int index)
{
	if_unlikely (ml == NULL)
		return -1;

	return ml_multiload_container_add_separator (ml->container, size, index);
}

bool
multiload_element_remove (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return false;

	return ml_multiload_container_element_remove (ml->container, index);
}

int
multiload_get_element_count (Multiload *ml)
{
	if_unlikely (ml == NULL)
		return -1;

	return ml_multiload_container_get_element_count (ml->container);
}

int
multiload_get_element_index_at_coords (Multiload *ml, int x, int y)
{
	if_unlikely (ml == NULL)
		return -1;

	return ml_multiload_container_get_element_index_at_coords (ml->container, x, y);
}

bool
multiload_element_is_graph (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return false;

	return ml_multiload_container_element_get_type (ml->container, index) == ML_MULTILOAD_ELEMENT_TYPE_GRAPH;
}

bool
multiload_element_is_separator (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return false;

	return ml_multiload_container_element_get_type (ml->container, index) == ML_MULTILOAD_ELEMENT_TYPE_SEPARATOR;
}

bool
multiload_element_move_back (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return false;

	if (index == 0)
		return false;

	int n = multiload_get_element_count(ml);
	if (n < 0)
		return false;

	return multiload_element_move_to (ml, index, index-1);
}

bool
multiload_element_move_forward (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return false;

	int n = multiload_get_element_count(ml);
	if (n < 0)
		return false;

	if (index == n-1)
		return false;

	return multiload_element_move_to (ml, index, index+1);
}

bool
multiload_element_move_to (Multiload *ml, int index, int new_index)
{
	if_unlikely (ml == NULL)
		return false;

	return ml_multiload_container_element_move (ml->container, index, new_index);
}

int
multiload_element_get_size (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return -1;

	return ml_multiload_container_element_get_size (ml->container, index);
}

bool
multiload_element_set_size (Multiload *ml, int index, int newsize)
{
	if_unlikely (ml == NULL)
		return false;

	return ml_multiload_container_element_set_size (ml->container, index, newsize);
}

const char *
multiload_element_get_name (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return NULL;

	if (multiload_element_is_separator(ml, index))
		return ML_ELEMENT_SEPARATOR_NAME;

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return NULL;

	return ml_graph_get_name (g);
}

const char *
multiload_element_get_label (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return NULL;

	if (multiload_element_is_separator(ml, index))
		return _("Separator");

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return NULL;

	return ml_graph_get_label (g);
}

void
multiload_set_failure_handler (Multiload *ml, MultiloadGraphFailureHandler handler, void* user_data)
{
	if_unlikely (ml == NULL)
		return;

	ml->failure_handler = handler;
	ml->failure_handler_data = user_data;
}

MultiloadGraphType *
multiload_list_graph_types ()
{
	MultiloadGraphType *list = ml_new_n (MultiloadGraphType, ML_GRAPH_TYPE_MAX+1);

	for (int i=0; i<ML_GRAPH_TYPE_MAX+1; i++) {
		const MlGraphTypeInterface *iface = ml_graph_type_interface_get ((MlGraphType)i);
		if_unlikely (iface == NULL)
			continue;

		list[i].name = iface->name;
		list[i].label = iface->label;
		list[i].description = iface->description;
		list[i].helptext = iface->helptext;
	}

	return list;
}

char *
multiload_list_graph_types_json ()
{
	cJSON *obj = cJSON_CreateObject();

	for (int i=0; i<ML_GRAPH_TYPE_MAX+1; i++) {
		const MlGraphTypeInterface *iface = ml_graph_type_interface_get ((MlGraphType)i);
		if_unlikely (iface == NULL)
			continue;

		cJSON *obj_child = cJSON_CreateObject();

		cJSON_AddStringToObject(obj_child, "name", 			ml_string_null_to_empty(iface->name));
		cJSON_AddStringToObject(obj_child, "label", 		ml_string_null_to_empty(iface->label));
		cJSON_AddStringToObject(obj_child, "description", 	ml_string_null_to_empty(iface->description));
		cJSON_AddStringToObject(obj_child, "helptext", 		ml_string_null_to_empty(iface->helptext));

		cJSON_AddItemToObject(obj, iface->name, obj_child);
	}

	char *ret = cJSON_Print (obj);

	cJSON_Delete (obj);
	return ret;
}

uint32_t
multiload_graph_get_ceiling (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return 0;

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return 0;

	return ml_graph_get_ceiling (g);
}

bool
multiload_graph_set_ceiling (Multiload *ml, int index, uint32_t ceiling)
{
	if_unlikely (ml == NULL)
		return false;

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return false;

	return ml_graph_set_ceiling (g, ceiling);
}

int
multiload_graph_get_border_size (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return -1;

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return -1;

	return ml_graph_get_border_size (g);
}

bool
multiload_graph_get_collect_stats (Multiload *ml, int index, int64_t *success_count, int64_t *fail_count)
{
	if_unlikely (ml == NULL)
		return false;

	if_unlikely (success_count == NULL && fail_count == NULL)
		return true;

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return false;

	int64_t s = ml_graph_get_collect_success_count (g);
	int64_t f = ml_graph_get_collect_fail_count (g);

	if (s >= 0 && f >= 0) {
		if (success_count != NULL)
			*success_count = s;

		if (fail_count != NULL)
			*fail_count = f;

		return true;
	} else {
		return false;
	}
}

bool
multiload_graph_last_collect_succeeded (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return false;

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return false;

	return ml_graph_get_last_collect_succeeded (g);
}

bool
multiload_graph_get_caption (Multiload *ml, int index, MultiloadGraphCaption *out)
{
	if_unlikely (ml == NULL || out == NULL)
		return false;

	if (!multiload_graph_is_running (ml, index)) {
		memset (out, 0, sizeof (MultiloadGraphCaption));
		out->body = _("This graph is paused");
		return true;
	}

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if_unlikely (g == NULL)
		return NULL;

	ml_graph_update_caption (g);

	MlCaption *caption = ml_graph_get_caption (g);
	if_unlikely (caption == NULL)
		return false;

	out->header = ml_caption_get (caption, ML_CAPTION_COMPONENT_HEADER);
	out->body   = ml_caption_get (caption, ML_CAPTION_COMPONENT_BODY);
	out->footer = ml_caption_get (caption, ML_CAPTION_COMPONENT_FOOTER);

	for (int col = 0; col < 2; col++)
		for (int row = 0; row < 3; row++)
			out->table_data[row][col] = ml_caption_get_table_data (caption, row, col);

	return true;
}

char *
multiload_graph_get_caption_json (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return NULL;

	if (!multiload_graph_is_running (ml, index))
		return ml_strdup_printf ("{ \"body\": \"%s\" }", _("This graph is paused"));

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if_unlikely (g == NULL)
		return NULL;

	ml_graph_update_caption (g);

	MlCaption *caption = ml_graph_get_caption (g);
	if_unlikely (caption == NULL)
		return NULL;

	cJSON *obj = ml_caption_to_json (caption);
	if (obj == NULL)
		return NULL;

	char *ret = cJSON_Print (obj);
	cJSON_Delete (obj);

	return ret;
}

bool
multiload_graph_set_border_size (Multiload *ml, int index, int border_size)
{
	if_unlikely (ml == NULL)
		return false;

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return false;

	if (!ml_graph_set_border_size (g, border_size))
		return false;

	ml_multiload_container_set_dirty (ml->container);
	return true;
}

const char *
multiload_graph_get_config (Multiload *ml, int index, const char *key)
{
	if_unlikely (ml == NULL)
		return NULL;

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return NULL;

	return ml_graph_get_config (g, key);
}

bool
multiload_graph_set_config (Multiload *ml, int index, const char *key, const char *value)
{
	if_unlikely (ml == NULL)
		return false;

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return false;

	return ml_graph_set_config (g, key, value);
}

MultiloadConfigEntry *
multiload_graph_list_config_entries (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return NULL;

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return NULL;

	MlConfig *conf = ml_graph_get_ml_config (g);
	if_unlikely (conf == NULL)
		return NULL;

	const char * const * keys = ml_config_list_keys (conf);
	int len = ml_strv_length ((char**)keys);
	if (len < 1) {
		free ((void*)keys);
		return NULL;
	}

	MultiloadConfigEntry *list = ml_new_n (MultiloadConfigEntry, len+1);

	for (int i = 0; i < len; i++) {
		list[i].key = keys[i];

		list[i].label = ml_config_get_label (conf, keys[i]);
		list[i].description = ml_config_get_description (conf, keys[i]);

		int64_t n[2];
		list[i].has_bounds = ml_config_get_entry_bounds (conf, keys[i], &n[0], &n[1]);

		MlValueType type = ml_config_get_entry_type (conf, keys[i]);
		switch (type) {
			case ML_VALUE_TYPE_INT32:
				list[i].type = 'i';
				if (list[i].has_bounds) {
					list[i].bounds[0] = n[0];
					list[i].bounds[1] = n[1];
				}
				break;

			case ML_VALUE_TYPE_INT64:
				list[i].type = 'I';
				if (list[i].has_bounds) {
					list[i].bounds[0] = n[0];
					list[i].bounds[1] = n[1];
				}
				break;

			case ML_VALUE_TYPE_TRISTATE:
				list[i].type = 't';
				break;

			case ML_VALUE_TYPE_BOOLEAN:
				list[i].type = 'b';
				break;

			case ML_VALUE_TYPE_STRING:
				list[i].type = 's';
				break;

			case ML_VALUE_TYPE_STRV:
				list[i].type = 'l';
				break;

			default:
				ml_error ("Unknown type for key '%s' (%d)", keys[i], (int)type);
				list[i].type = '?';
				break;
		}
	}

	free ((void*)keys);
	return list;
}

char *
multiload_graph_list_config_entries_json (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return NULL;

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return NULL;

	MlConfig *conf = ml_graph_get_ml_config (g);
	if_unlikely (conf == NULL)
		return NULL;

	const char * const * keys = ml_config_list_keys (conf);
	int len = ml_strv_length ((char**)keys);
	if (len < 1) {
		free ((void*)keys);
		return NULL;
	}

	cJSON *obj = cJSON_CreateObject();

	for (int i = 0; i < len; i++) {
		cJSON *obj_item = cJSON_CreateObject();
		cJSON_AddStringToObject (obj_item, "label",			ml_string_null_to_empty (ml_config_get_label (conf, keys[i])));
		cJSON_AddStringToObject (obj_item, "description",	ml_string_null_to_empty (ml_config_get_description (conf, keys[i])));

		int64_t n[2];
		bool has_bounds = ml_config_get_entry_bounds (conf, keys[i], &n[0], &n[1]);

		MlValueType type = ml_config_get_entry_type (conf, keys[i]);
		switch (type) {
			case ML_VALUE_TYPE_INT32:
				cJSON_AddStringToObject (obj_item, "type", "i");
				if (has_bounds) {
					cJSON_AddNumberToObject (obj_item, "min", n[0]);
					cJSON_AddNumberToObject (obj_item, "max", n[1]);
				}
				break;

			case ML_VALUE_TYPE_INT64:
				cJSON_AddStringToObject (obj_item, "type", "I");
				if (has_bounds) {
					cJSON_AddNumberToObject (obj_item, "min", n[0]);
					cJSON_AddNumberToObject (obj_item, "max", n[1]);
				}
				break;

			case ML_VALUE_TYPE_TRISTATE:
				cJSON_AddStringToObject (obj_item, "type", "t");
				break;

			case ML_VALUE_TYPE_BOOLEAN:
				cJSON_AddStringToObject (obj_item, "type", "b");
				break;

			case ML_VALUE_TYPE_STRING:
				cJSON_AddStringToObject (obj_item, "type", "s");
				break;

			case ML_VALUE_TYPE_STRV:
				cJSON_AddStringToObject (obj_item, "type", "l");
				break;

			default:
				ml_error ("Unknown type for key '%s' (%d)", keys[i], (int)type);
				cJSON_AddStringToObject (obj_item, "type", "?");
				break;
		}

		cJSON_AddItemToObject (obj, keys[i], obj_item);
	}

	free ((void*)keys);

	char *ret = cJSON_Print (obj);

	cJSON_Delete (obj);
	return ret;
}

char *
multiload_graph_get_style_json (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return NULL;

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return NULL;

	const MlGraphStyle *style = ml_graph_get_style (g);
	if (style == NULL)
		return NULL;

	cJSON *obj = ml_graph_style_to_json (style);
	char *ret = cJSON_Print (obj);

	cJSON_Delete (obj);
	return ret;
}

bool
multiload_graph_set_style_json (Multiload *ml, int index, const char *style_json)
{
	if_unlikely (ml == NULL || style_json == NULL)
		return false;

	const char *parse_end;
	cJSON *obj = cJSON_ParseWithOpts(style_json, &parse_end, 1);
	if (obj == NULL) {
		ml_error ("Malformed JSON data near character %zu", (size_t)(parse_end - style_json));
		return false;
	}

	MlGraph *g = ml_multiload_container_get_graph (ml->container, index);
	if (g == NULL)
		return false;

	MlGraphStyle *style = ml_graph_style_new_from_json (obj);
	cJSON_Delete (obj);

	if (style == NULL)
		return false;

	if (!ml_graph_set_style (g, style)) {
		ml_graph_style_destroy (style);
		return false;
	}

	return true;
}

bool
multiload_graph_start (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return false;

	return ml_multiload_container_graph_start (ml->container, index);
}

bool
multiload_graph_stop (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return false;

	return ml_multiload_container_graph_stop (ml->container, index);
}

bool
multiload_graph_is_running (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return false;

	return ml_multiload_container_graph_is_running (ml->container, index);
}

int
multiload_graph_get_interval (Multiload *ml, int index)
{
	if_unlikely (ml == NULL)
		return -1;

	return ml_multiload_container_graph_get_interval (ml->container, index);
}

bool
multiload_graph_set_interval (Multiload *ml, int index, int interval_ms)
{
	if_unlikely (ml == NULL)
		return false;

	return ml_multiload_container_graph_set_interval (ml->container, index, interval_ms);
}

void
multiload_wait_for_data (Multiload *ml)
{
	if_unlikely (ml == NULL)
		return;

	ml_multiload_container_wait_for_data (ml->container);
}

bool
multiload_needs_redraw (Multiload *ml)
{
	if_unlikely (ml == NULL)
		return false;

	return ml_multiload_container_needs_redraw (ml->container);
}

uint8_t *
multiload_write_to_png_buffer (Multiload *ml, size_t *len_ptr, int compression_level)
{
	if_unlikely (ml == NULL || len_ptr == NULL)
		return false;

	// update surface
	ml_multiload_container_draw (ml->container);

	MlSurface *s = ml_multiload_container_get_surface (ml->container);
	if_unlikely (s == NULL)
		return false;

	return ml_surface_write_to_png_buffer (s, len_ptr, compression_level);
}

bool
multiload_write_to_png_file (Multiload *ml, const char *filename, int compression_level)
{
	if_unlikely (ml == NULL)
		return false;

	// update surface
	ml_multiload_container_draw (ml->container);

	MlSurface *s = ml_multiload_container_get_surface (ml->container);
	if_unlikely (s == NULL)
		return false;

	return ml_surface_write_to_png_file (s, filename, compression_level);
}

uint8_t *
multiload_get_surface_data (Multiload *ml, int *width, int *height)
{
	if_unlikely (ml == NULL || width == NULL || height == NULL)
		return NULL;

	// update surface
	ml_multiload_container_draw (ml->container);

	MlSurface *s = ml_multiload_container_get_surface (ml->container);
	if_unlikely (s == NULL)
		return NULL;

	uint8_t *data = ml_surface_get_data (s, width, height);
	if_unlikely (data == NULL)
		return NULL;

	return data;
}

char *
multiload_to_json (Multiload *ml)
{
	if_unlikely (ml == NULL || ml->container == NULL)
		return NULL;

	cJSON* obj = cJSON_CreateObject ();

	cJSON_AddStringToObject (obj, "product", "Multiload-ng");
	cJSON_AddStringToObject (obj, "version", PACKAGE_VERSION);
	cJSON_AddItemToObject (obj, "shared-config", ml_config_to_json (ML_SHARED_GET (config)));
	cJSON_AddItemToObject (obj, "container", ml_multiload_container_to_json (ml->container));

	char *json_str = cJSON_Print (obj);
	cJSON_Delete (obj);

	return json_str;
}

bool
multiload_to_json_file (Multiload *ml, const char *path)
{
	if_unlikely (ml == NULL)
		return false;

	char *json_str = multiload_to_json (ml);
	if_unlikely (json_str == NULL)
		return false;

	FILE *f = fopen (path, "wb");
	if (f == NULL) {
		ml_warning ("Cannot open '%s' for writing: %s", path, strerror (errno));
		free (json_str);
		return false;
	}

	int result = fputs (json_str, f);
	free (json_str);
	fclose (f);

	if (result == EOF) {
		ml_error ("Cannot write data to '%s': %s", path, strerror (errno));
		return false;
	}

	return true;
}

bool
multiload_debug_to_zip (Multiload *ml, const char *filename, int compression_level)
{
	bool ret = ml_debug_collect_to_zip (filename, compression_level);
	(void)ml; // TODO also include multiload_to_json
	return ret;
}

const char *
multiload_version() {
	return PACKAGE_VERSION;
}
