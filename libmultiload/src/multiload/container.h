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

#ifndef ML_HEADER__MULTILOAD_CONTAINER_H__INCLUDED
#define ML_HEADER__MULTILOAD_CONTAINER_H__INCLUDED
ML_HEADER_BEGIN


typedef enum {
	ML_MULTILOAD_ELEMENT_TYPE_GRAPH,
	ML_MULTILOAD_ELEMENT_TYPE_SEPARATOR
} MlMultiloadElementType;

typedef struct _MlMultiloadContainer MlMultiloadContainer;


MlMultiloadContainer *
ml_multiload_container_new (int size, MlOrientation orientation, int padding);

MlMultiloadContainer *
ml_multiload_container_new_from_json (cJSON *obj, const char *theme_name);

void
ml_multiload_container_destroy (MlMultiloadContainer *container);

size_t
ml_multiload_container_sizeof (MlMultiloadContainer *container)
ML_FN_SIZEOF;

int
ml_multiload_container_get_size (MlMultiloadContainer *container)
ML_FN_READ_PROPERTY;

bool
ml_multiload_container_set_size (MlMultiloadContainer *container, int newsize)
ML_FN_WARN_UNUSED_RESULT;

int
ml_multiload_container_get_length (MlMultiloadContainer *container)
ML_FN_READ_PROPERTY;

int
ml_multiload_container_get_padding (MlMultiloadContainer *container)
ML_FN_READ_PROPERTY;

bool
ml_multiload_container_set_padding (MlMultiloadContainer *container, int newpadding)
ML_FN_WARN_UNUSED_RESULT;

MlOrientation
ml_multiload_container_get_orientation (MlMultiloadContainer *container)
ML_FN_READ_PROPERTY;

bool
ml_multiload_container_set_orientation (MlMultiloadContainer *container, MlOrientation neworient)
ML_FN_WARN_UNUSED_RESULT;

MlSurface *
ml_multiload_container_get_surface (MlMultiloadContainer *container)
ML_FN_READ_PROPERTY;

void
ml_multiload_container_set_graph_fail_function (MlMultiloadContainer *container, MlGraphFailFunc fn, mlPointer user_data);

int
ml_multiload_container_get_element_count (MlMultiloadContainer *container)
ML_FN_READ_PROPERTY;

MlMultiloadElementType
ml_multiload_container_element_get_type (MlMultiloadContainer *container, int index)
ML_FN_READ_PROPERTY;

MlGraph *
ml_multiload_container_get_graph (MlMultiloadContainer *container, int index)
ML_FN_READ_PROPERTY;

int
ml_multiload_container_add_graph (MlMultiloadContainer *container, MlGraphType type, int size, int border_size, MlGraphStyle *style, int interval_ms, int index);

int
ml_multiload_container_add_separator (MlMultiloadContainer *container, int size, int index);

bool
ml_multiload_container_element_remove (MlMultiloadContainer *container, int index)
ML_FN_WARN_UNUSED_RESULT;

bool
ml_multiload_container_element_move (MlMultiloadContainer *container, int index, int new_index)
ML_FN_WARN_UNUSED_RESULT;

int
ml_multiload_container_get_element_index_at_coords (MlMultiloadContainer *container, int x, int y)
ML_FN_READ_PROPERTY;

int
ml_multiload_container_element_get_size (MlMultiloadContainer *container, int index)
ML_FN_READ_PROPERTY;

bool
ml_multiload_container_element_set_size (MlMultiloadContainer *container, int index, int newsize)
ML_FN_WARN_UNUSED_RESULT;

int
ml_multiload_container_graph_get_interval (MlMultiloadContainer *container, int index)
ML_FN_READ_PROPERTY;

bool
ml_multiload_container_graph_set_interval (MlMultiloadContainer *container, int index, int interval_ms)
ML_FN_WARN_UNUSED_RESULT;

int
ml_multiload_container_graph_find_index (MlMultiloadContainer *container, MlGraph *g);

bool
ml_multiload_container_graph_start (MlMultiloadContainer *container, int index)
ML_FN_WARN_UNUSED_RESULT;

bool
ml_multiload_container_graph_stop (MlMultiloadContainer *container, int index)
ML_FN_WARN_UNUSED_RESULT;

bool
ml_multiload_container_graph_is_running (MlMultiloadContainer *container, int index)
ML_FN_READ_PROPERTY;

void
ml_multiload_container_wait_for_data (MlMultiloadContainer *container);

void
ml_multiload_container_set_dirty (MlMultiloadContainer *container);

bool
ml_multiload_container_needs_redraw (MlMultiloadContainer *container)
ML_FN_READ_PROPERTY;

void
ml_multiload_container_draw (MlMultiloadContainer *container);

cJSON *
ml_multiload_container_element_to_json (MlMultiloadContainer *container, int index)
ML_FN_COLD;

cJSON *
ml_multiload_container_to_json (MlMultiloadContainer *container)
ML_FN_COLD;


ML_HEADER_END
#endif /* ML_HEADER__MULTILOAD_CONTAINER_H__INCLUDED */
