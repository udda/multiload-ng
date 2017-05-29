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


typedef struct {
	MlMultiloadElementType type;
	int size;

	// the following elements are NULL if type == ML_MULTILOAD_ELEMENT_TYPE_SEPARATOR
	MlGraph *graph;
	MlMultiloadTimer *timer;
} MlMultiloadElement;


struct _MlMultiloadContainer {
	MlGrowArray *elements;
	MlSurface *surface;

	int size; // includes padding
	int padding;
	MlOrientation orientation;

	bool is_dirty;

	MlMultiloadNotifier *notifier;

	MlGraphFailFunc fail_fn;
	mlPointer fail_fn_data;
};


static inline void
_ml_multiload_container_graph_fail_relay (MlGraph *g, int fail_code, const char *fail_msg, MlMultiloadContainer *container)
{
	if_unlikely (container == NULL || container->fail_fn == NULL)
		return;

	container->fail_fn (g, fail_code, fail_msg, container->fail_fn_data);
}

static inline bool
_ml_multiload_container_check_index (MlMultiloadContainer *container, int index, bool create)
{
	if_unlikely (container == NULL || container->elements == NULL)
		return false;

	int len = (int)ml_grow_array_get_length (container->elements);

	if (create) {
		// in "create" situations, negative index means "auto", index adjacent to the last element is also allowed
		if (index <= len) {
			return true;
		} else {
			ml_warning ("Element index (%d) is out of 'create' bounds ([-1, %u])", index, len);
			return false;
		}
	} else {
		// in "get" situations, negative index is invalid, and index must be existing
		if (len == 0) {
			ml_warning ("No elements in container, cannot get element %d", index);
			return false;
		}

		if (index >= 0 || index < (int)len) {
			return true;
		} else {
			ml_warning ("Element index (%d) is out of 'get' bounds ([0, %u])", index, len-1);
			return false;
		}
	}
}

static inline MlMultiloadElement*
_ml_multiload_container_get_element (MlMultiloadContainer *container, int index)
{
	if_unlikely (container == NULL || !_ml_multiload_container_check_index(container, index, false))
		return NULL;

	return (MlMultiloadElement*)ml_grow_array_get (container->elements, index);
}

static int
_ml_multiload_container_add_element (MlMultiloadContainer *container, MlMultiloadElementType type, MlGraph *graph, MlMultiloadTimer *timer, int size, int index)
{
	if_unlikely (container == NULL || size <= 0)
		return -1;

	if_unlikely (!_ml_multiload_container_check_index(container, index, true)) 
		return -1;

	if_unlikely (type == ML_MULTILOAD_ELEMENT_TYPE_GRAPH && (graph == NULL || timer == NULL)) {
		ml_error ("Element type is graph, but graph reference is NULL");
		return -1;
	}

	if_unlikely (ml_grow_array_get_length (container->elements) >= ML_CONTAINER_MAX_ELEMENTS) {
		ml_error ("Too many elements in container");
		return -1;
	}

	MlMultiloadElement *e = ml_new (MlMultiloadElement);
	e->type = type;
	e->size = size;
	e->graph = graph;
	e->timer = timer;

	container->is_dirty = true;

	if (index < 0) {
		ml_grow_array_append (container->elements, e);
		return ml_grow_array_get_length (container->elements) - 1;
	} else {
		ml_grow_array_insert (container->elements, e, index);
		return index;
	}
}

static inline int
_ml_multiload_container_get_available_size (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL)
		return -1;

	return container->size - 2 * container->padding;
}

static bool
_ml_multiload_container_refresh_element_size (MlMultiloadContainer *container, int index)
{
	if_unlikely (container == NULL)
		return false;

	MlMultiloadElement *e = _ml_multiload_container_get_element (container, index);
	if_unlikely (e == NULL)
		return false;

	int available_size = _ml_multiload_container_get_available_size (container);

	if (e->type == ML_MULTILOAD_ELEMENT_TYPE_GRAPH && e->graph != NULL) {
		return ml_graph_resize (
			e->graph,
			ml_orientation_x (container->orientation, e->size, available_size),
			ml_orientation_y (container->orientation, e->size, available_size)
		);
	}

	return true;
}

static inline bool
_ml_multiload_container_check_dimensions (int size, int padding)
{
	if (size <= 0) {
		ml_warning ("Invalid size specified (%d)", size);
		return false;
	}

	if (padding < 0) {
		ml_warning ("Invalid padding specified (%d)", padding);
		return false;
	}

	if (size <= 2*padding) {
		ml_warning ("Size too small (%d) for specified padding (%d)", size, padding);
		return false;
	}
	return true;
}

MlMultiloadContainer*
ml_multiload_container_new (int size, MlOrientation orientation, int padding)
{
	if_unlikely (!_ml_multiload_container_check_dimensions (size, padding))
		return NULL;

	MlMultiloadContainer *container = ml_new (MlMultiloadContainer);
	container->elements = ml_grow_array_new (10, (MlDestroyFunc)free);
	container->surface = ml_surface_new (1, 1);

	container->size = size;
	container->orientation = orientation;
	container->padding = padding;
	container->is_dirty = true;

	container->notifier = ml_multiload_notifier_new ();
	return container;
}

MlMultiloadContainer *
ml_multiload_container_new_from_json (cJSON *obj, const char *theme_name)
{
	if_unlikely (obj == NULL)
		return NULL;

	cJSON *obj_size = ml_cJSON_GetObjectItem (obj, "size");
	if (obj_size == NULL)
		return NULL;
	int container_size = obj_size->valueint;

	cJSON *obj_padding = ml_cJSON_GetObjectItem (obj, "padding");
	if (obj_padding == NULL)
		return NULL;
	int container_padding = obj_padding->valueint;

	MlOrientation container_orientation = ml_orientation_parse_json (ml_cJSON_GetObjectItem (obj, "orientation"));

	MlMultiloadContainer *container = ml_multiload_container_new (container_size, container_orientation, container_padding);
	if (container == NULL)
		return NULL;

	cJSON *obj_elements = ml_cJSON_GetObjectItem (obj, "elements");
	if (obj_elements == NULL || !cJSON_IsArray(obj_elements)) {
		ml_multiload_container_destroy (container);
		return NULL;
	}

	int available_size = _ml_multiload_container_get_available_size (container);

	for (int i = 0; i < cJSON_GetArraySize(obj_elements); i++) {
		cJSON *obj_item = cJSON_GetArrayItem (obj_elements, i);
		if_unlikely (obj_item == NULL)
			continue;
	
		cJSON *obj_type = ml_cJSON_GetObjectItem (obj_item, "type");
		if (obj_type == NULL)
			continue;

		cJSON *obj_size = ml_cJSON_GetObjectItem (obj_item, "size");
		if (obj_size == NULL)
			continue;

		int element_size = obj_size->valueint;

		if (ml_string_equals (obj_type->valuestring, "separator", false)) {
			ml_multiload_container_add_separator (container, element_size, -1);
		} else if (ml_string_equals (obj_type->valuestring, "graph", false)) {

			cJSON *obj_interval = ml_cJSON_GetObjectItem (obj_item, "interval");
			if (obj_interval == NULL)
				continue;

			cJSON *obj_graph = ml_cJSON_GetObjectItem (obj_item, "graph");
			if (obj_graph == NULL)
				continue;

			MlGraph *g = ml_graph_new_from_json (
				obj_graph,
				ml_orientation_x (container_orientation, element_size, available_size),
				ml_orientation_y (container_orientation, element_size, available_size),
				theme_name
				);
			if (g == NULL) {
				ml_warning ("Cannot create graph element %d from JSON", i);
				continue;
			}

			MlMultiloadTimer *timer = ml_multiload_timer_new (container->notifier, g, obj_interval->valueint);
			ml_graph_set_fail_function (g, (MlGraphFailFunc)_ml_multiload_container_graph_fail_relay, container);

			int ret = _ml_multiload_container_add_element (container, ML_MULTILOAD_ELEMENT_TYPE_GRAPH, g, timer, element_size, -1);
			if_unlikely (ret < 0 || !ml_multiload_timer_start (timer)) {
				ml_graph_destroy (g);
				ml_multiload_timer_destroy (timer);
				ml_warning ("Cannot start graph element %d", i);
				continue;
			}
		} else {
			ml_warning ("Unsupported element type: %s", obj_type->valuestring);
			continue;
		}
	}

	return container;
}

void
ml_multiload_container_destroy (MlMultiloadContainer *container)
{
	if_likely (container != NULL) {
		for (int i = ml_grow_array_get_length(container->elements)-1; i >= 0; i--) {

			MlMultiloadElement *e = _ml_multiload_container_get_element (container, i);
			if_unlikely (e == NULL) {
				ml_bug ("Missing element %d", i);
				continue;
			}

			if (e->type == ML_MULTILOAD_ELEMENT_TYPE_GRAPH) {
				ml_multiload_timer_destroy (e->timer);
				ml_graph_destroy (e->graph);
			}
		}
		ml_grow_array_destroy (container->elements, true, false);

		ml_surface_destroy (container->surface);
		ml_multiload_notifier_destroy (container->notifier);

		free (container);
	}
}

size_t
ml_multiload_container_sizeof (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL || container->elements == NULL)
		return 0;

	size_t size = sizeof (MlMultiloadContainer);
	size += ml_surface_sizeof (container->surface);
	size += ml_multiload_notifier_sizeof (container->notifier);

	// elements
	ml_grow_array_for (container->elements, i) {
		size += sizeof (MlMultiloadElement);

		MlMultiloadElement *e = _ml_multiload_container_get_element (container, i);
		if (e != NULL) {
			if (e->graph != NULL)
				size += ml_graph_sizeof (e->graph);
			if (e->timer != NULL)
				size += ml_multiload_timer_sizeof (e->timer);
		}
	}

	return size;
}

int
ml_multiload_container_get_size (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL)
		return -1;

	return container->size;
}

bool
ml_multiload_container_set_size (MlMultiloadContainer *container, int newsize)
{
	if_unlikely (container == NULL)
		return false;

	if_unlikely (!_ml_multiload_container_check_dimensions (newsize, container->padding))
		return false;

	container->size = newsize;

	ml_grow_array_for (container->elements, i) {
		if_unlikely (!_ml_multiload_container_refresh_element_size (container, i))
			return false;
	}

	container->is_dirty = true;

	return true;
}

int
ml_multiload_container_get_length (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL)
		return -1;

	int len = 0;
	ml_grow_array_for (container->elements, i) {
		MlMultiloadElement *e = _ml_multiload_container_get_element (container, i);
		if_likely (e != NULL)
			len += e->size;
	}

	return len;
}

int
ml_multiload_container_get_padding (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL)
		return -1;

	return container->padding;
}

bool
ml_multiload_container_set_padding (MlMultiloadContainer *container, int newpadding)
{
	if_unlikely (container == NULL)
		return false;

	if_unlikely (!_ml_multiload_container_check_dimensions (container->size, newpadding))
		return false;

	container->padding = newpadding;

	ml_grow_array_for (container->elements, i) {
		if_unlikely (!_ml_multiload_container_refresh_element_size (container, i))
			return false;
	}

	container->is_dirty = true;

	return true;
}

MlOrientation
ml_multiload_container_get_orientation (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL)
		return ML_INVALID;

	return container->orientation;
}

bool
ml_multiload_container_set_orientation (MlMultiloadContainer *container, MlOrientation neworient)
{
	if_unlikely (container == NULL)
		return false;

	container->orientation = neworient;

	ml_grow_array_for (container->elements, i) {
		if_unlikely (!_ml_multiload_container_refresh_element_size (container, i))
			return false;
	}

	container->is_dirty = true;

	return true;
}

MlSurface*
ml_multiload_container_get_surface (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL)
		return NULL;

	return container->surface;
}

void
ml_multiload_container_set_graph_fail_function (MlMultiloadContainer *container, MlGraphFailFunc fn, mlPointer user_data)
{
	if_unlikely (container == NULL)
		return;

	container->fail_fn = fn;
	container->fail_fn_data = user_data;
}

int
ml_multiload_container_get_element_count (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL || container->elements == NULL)
		return -1;

	return (int)ml_grow_array_get_length (container->elements);
}

MlMultiloadElementType
ml_multiload_container_element_get_type (MlMultiloadContainer *container, int index)
{
	if_unlikely (container == NULL || container->elements == NULL)
		return ML_INVALID;

	MlMultiloadElement *e = _ml_multiload_container_get_element (container, index);
	if_unlikely (e == NULL)
		return ML_INVALID;

	return e->type;
}

MlGraph *
ml_multiload_container_get_graph (MlMultiloadContainer *container, int index)
{
	if_unlikely (container == NULL)
		return NULL;

	MlMultiloadElement *e = _ml_multiload_container_get_element (container, index);
	if_unlikely (e == NULL)
		return NULL;

	if (e->type != ML_MULTILOAD_ELEMENT_TYPE_GRAPH) {
		ml_warning ("Element %d is not a graph", index);
		return NULL;
	}

	if (e->graph == NULL)
		ml_warning ("Null graph reference");

	return e->graph;
}

int
ml_multiload_container_add_graph (MlMultiloadContainer *container, MlGraphType type, int size, int border_size, MlGraphStyle *style, int interval_ms, int index)
{
	if_unlikely (container == NULL)
		return -1;

	int available_size = _ml_multiload_container_get_available_size (container);

	MlGraph *g = ml_graph_new (
		type,
		ml_orientation_x (container->orientation, size, available_size),
		ml_orientation_y (container->orientation, size, available_size),
		border_size,
		style
	);

	MlMultiloadTimer *timer = ml_multiload_timer_new (container->notifier, g, interval_ms);

	int ret = _ml_multiload_container_add_element (container, ML_MULTILOAD_ELEMENT_TYPE_GRAPH, g, timer, size, index); // container->is_dirty is set there;

	if_unlikely (ret < 0 || !ml_multiload_timer_start (timer)) {
		ml_graph_destroy (g);
		ml_multiload_timer_destroy (timer);
		return -1;
	}

	ml_graph_set_fail_function (g, (MlGraphFailFunc)_ml_multiload_container_graph_fail_relay, container);

	return ret;
}

int
ml_multiload_container_add_separator (MlMultiloadContainer *container, int size, int index)
{
	return _ml_multiload_container_add_element (container, ML_MULTILOAD_ELEMENT_TYPE_SEPARATOR, NULL, NULL, size, index); // container->is_dirty is set there;
}

bool
ml_multiload_container_element_move (MlMultiloadContainer *container, int index, int new_index)
{
	if_unlikely (container == NULL || container->elements == NULL)
		return false;

	if (new_index < 0 || new_index >= (int)ml_grow_array_get_length (container->elements))
		return false;

	ml_grow_array_move (container->elements, index, new_index);

	container->is_dirty = true;
	return true;
}

int
ml_multiload_container_get_element_index_at_coords (MlMultiloadContainer *container, int x, int y)
{
	if_unlikely (container == NULL || x < 0 || y < 0)
		return -1;

	int l;
	if (container->orientation == ML_ORIENTATION_HORIZONTAL) {
		if (y < container->padding || y >= container->size + container->padding)
			return -1;
		l = x;
	} else { // ML_ORIENTATION_VERTICAL
		if (x < container->padding || x >= container->size + container->padding)
			return -1;
		l = y;
	}

	int s = 0;

	ml_grow_array_for (container->elements, i) {
		MlMultiloadElement *e = _ml_multiload_container_get_element (container, i);
		if_unlikely (e == NULL) {
			ml_bug ("Missing element %d", i);
			return -1;
		}

		s += e->size;
		if (l < s)
			return i;
	}

	return -1;
}

int
ml_multiload_container_element_get_size (MlMultiloadContainer *container, int index)
{
	if_unlikely (container == NULL || container->elements == NULL)
		return -1;

	MlMultiloadElement *e = _ml_multiload_container_get_element (container, index);
	if_unlikely (e == NULL)
		return -1;

	return e->size;
}

bool
ml_multiload_container_element_set_size (MlMultiloadContainer *container, int index, int newsize)
{
	if_unlikely (container == NULL)
		return false;

	MlMultiloadElement *e = _ml_multiload_container_get_element (container, index);
	if_unlikely (e == NULL)
		return false;

	int oldsize = e->size;
	e->size = newsize;

	if_unlikely (!_ml_multiload_container_refresh_element_size (container, index)) {
		e->size = oldsize;
		return false;
	}

	container->is_dirty = true;
	return true;
}

bool
ml_multiload_container_element_remove (MlMultiloadContainer *container, int index)
{
	if_unlikely (container == NULL || container->elements == NULL)
		return false;

	if (ml_multiload_container_get_element_count (container) <= 1)
		return false;

	MlMultiloadElement *e = _ml_multiload_container_get_element (container, index);
	if_unlikely (e == NULL)
		return false;

	if (e->type == ML_MULTILOAD_ELEMENT_TYPE_GRAPH) {
		ml_multiload_timer_destroy (e->timer);
		ml_graph_destroy (e->graph);
	}

	ml_grow_array_remove (container->elements, index);

	container->is_dirty = true;
	return true;
}

int
ml_multiload_container_graph_get_interval (MlMultiloadContainer *container, int index)
{
	if_unlikely (container == NULL)
		return -1;

	MlMultiloadElement *e = _ml_multiload_container_get_element (container, index);
	if_unlikely (e == NULL)
		return -1;

	return ml_multiload_timer_get_interval (e->timer);
}

bool
ml_multiload_container_graph_set_interval (MlMultiloadContainer *container, int index, int interval_ms)
{
	if_unlikely (container == NULL)
		return false;

	MlMultiloadElement *e = _ml_multiload_container_get_element (container, index);
	if_unlikely (e == NULL)
		return false;

	return ml_multiload_timer_set_interval (e->timer, interval_ms);
}

int
ml_multiload_container_graph_find_index (MlMultiloadContainer *container, MlGraph *g)
{
	if_unlikely (container == NULL || g == NULL)
		return -1;

	int index = -1;
	for (int i = 0; i < ml_multiload_container_get_element_count(container); i++) {
		MlMultiloadElement *e = _ml_multiload_container_get_element (container, i);
		if (e != NULL && e->graph == g) {
			index = i;
			break;
		}
	}

	return index;
}

bool
ml_multiload_container_graph_start (MlMultiloadContainer *container, int index)
{
	if_unlikely (container == NULL)
		return false;

	MlMultiloadElement *e = _ml_multiload_container_get_element (container, index);
	if_unlikely (e == NULL)
		return false;

	ml_graph_unpause (e->graph); // do the necessary steps (like deleting previous calculations) to avoid spikes after a resume
	container->is_dirty = true;
	return ml_multiload_timer_start (e->timer);
}

bool
ml_multiload_container_graph_stop (MlMultiloadContainer *container, int index)
{
	if_unlikely (container == NULL)
		return false;

	MlMultiloadElement *e = _ml_multiload_container_get_element (container, index);
	if_unlikely (e == NULL)
		return false;

	container->is_dirty = true;
	ml_multiload_timer_stop (e->timer);

	return true;
}

bool
ml_multiload_container_graph_is_running (MlMultiloadContainer *container, int index)
{
	if_unlikely (container == NULL)
		return false;

	MlMultiloadElement *e = _ml_multiload_container_get_element (container, index);
	if_unlikely (e == NULL)
		return false;

	return ml_multiload_timer_is_running (e->timer);
}

void
ml_multiload_container_wait_for_data (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL)
		return;

	ml_multiload_notifier_wait (container->notifier);
}

void
ml_multiload_container_set_dirty (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL)
		return;

	container->is_dirty = true;
}

bool
ml_multiload_container_needs_redraw (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL)
		return false;

	if (container->is_dirty)
		return true;

	return ml_multiload_notifier_check (container->notifier);
}

void
ml_multiload_container_draw (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL || container->elements == NULL || container->surface == NULL)
		return;

	// reset to 1x1 surface when there are no elements
	if (ml_grow_array_is_empty (container->elements)) {
		if (!ml_surface_resize (container->surface, 1, 1))
			ml_error ("Could not clear container surface. Surface may contain old data.");
		return;
	}

	int len = ml_multiload_container_get_length (container);
	if (!ml_surface_resize (	container->surface,
								ml_orientation_x (container->orientation, len, container->size),
								ml_orientation_y (container->orientation, len, container->size) )) {
		ml_error ("Could not resize container surface. Surface may contain old data.");
		return;
	}

	if (container->is_dirty) {
		// clear the surface, as reallocation might create dirty areas
		ml_surface_clear (container->surface);
		container->is_dirty = false;
	}

	// draw elements into surface
	len = 0;
	ml_grow_array_for (container->elements, i) {
		MlMultiloadElement *e = _ml_multiload_container_get_element (container, i);
		if (e == NULL)
			continue;

		if (e->type == ML_MULTILOAD_ELEMENT_TYPE_GRAPH) {
			ml_graph_draw (e->graph);
			
			MlSurface *graph_surface = ml_graph_get_surface (e->graph);

			if (!ml_multiload_timer_is_running (e->timer))
				ml_surface_desaturate (graph_surface);

			ml_surface_copy_surface (
				container->surface,
				ml_orientation_x (container->orientation, len, container->padding),
				ml_orientation_y (container->orientation, len, container->padding),
				graph_surface
			);
		}

		len += e->size;
	}
}

cJSON *
ml_multiload_container_element_to_json (MlMultiloadContainer *container, int index)
{
	if_unlikely (container == NULL || container->elements == NULL)
		return NULL;

	MlMultiloadElement *e = _ml_multiload_container_get_element (container, index);
	if_unlikely (e == NULL)
		return NULL;

	cJSON* obj = cJSON_CreateObject ();
	cJSON_AddNumberToObject (obj, "size", e->size);

	if (e->type == ML_MULTILOAD_ELEMENT_TYPE_GRAPH)
		cJSON_AddStringToObject (obj, "type", "graph");
	else if (e->type == ML_MULTILOAD_ELEMENT_TYPE_SEPARATOR)
		cJSON_AddStringToObject (obj, "type", "separator");
	else {
		ml_bug ("Unknown element type (code %d)", e->type);
		cJSON_Delete (obj);
		return NULL;
	}

	if (e->type == ML_MULTILOAD_ELEMENT_TYPE_GRAPH) {
		cJSON_AddNumberToObject (obj, "interval", ml_multiload_timer_get_interval (e->timer));
		cJSON_AddItemToObject (obj, "graph", ml_graph_to_json (e->graph));
	}

	return obj;
}


cJSON *
ml_multiload_container_to_json (MlMultiloadContainer *container)
{
	if_unlikely (container == NULL || container->elements == NULL)
		return NULL;

	cJSON* obj = cJSON_CreateObject ();

	cJSON_AddNumberToObject	(obj, "size",		 (container->size));
	cJSON_AddNumberToObject	(obj, "padding",	 (container->padding));
	cJSON_AddItemToObject	(obj, "orientation", ml_orientation_to_json(container->orientation));

	cJSON* obj_elements = cJSON_CreateArray ();
	ml_grow_array_for (container->elements, i) {
		cJSON *obj_item = ml_multiload_container_element_to_json (container, i);
		if_unlikely (obj_item == NULL)
			continue;

		cJSON_AddItemToArray (obj_elements, obj_item);
	}
	cJSON_AddItemToObject (obj, "elements", obj_elements);

	return obj;
}
