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

#ifndef ML_HEADER__GRAPH_GRAPH_H__INCLUDED
#define ML_HEADER__GRAPH_GRAPH_H__INCLUDED
ML_HEADER_BEGIN
#include "style.h"
#include "type.h"


typedef struct _MlGraph MlGraph;
typedef void (*MlGraphFailFunc) (MlGraph *g, int fail_code, const char *fail_msg, mlPointer user_data);


MlGraph *
ml_graph_new (MlGraphType type, int width, int height, int border_size, MlGraphStyle *style);

MlGraph *
ml_graph_new_from_json (cJSON *obj, int width, int height, const char *theme_name);

void
ml_graph_destroy (MlGraph *g);

size_t
ml_graph_sizeof (MlGraph *g)
ML_FN_SIZEOF;

bool
ml_graph_resize (MlGraph *g, int width, int height)
ML_FN_WARN_UNUSED_RESULT;

void
ml_graph_set_fail_function (MlGraph *g, MlGraphFailFunc func, mlPointer user_data);

void
ml_graph_unpause (MlGraph *g);

int64_t
ml_graph_get_collect_success_count (MlGraph *g)
ML_FN_READ_PROPERTY;

int64_t
ml_graph_get_collect_fail_count (MlGraph *g)
ML_FN_READ_PROPERTY;

bool
ml_graph_get_last_collect_succeeded (MlGraph *g)
ML_FN_READ_PROPERTY;

const char *
ml_graph_get_error_message (MlGraph *g)
ML_FN_READ_PROPERTY;

const char *
ml_graph_get_name (MlGraph *g)
ML_FN_READ_PROPERTY;

const char *
ml_graph_get_label (MlGraph *g)
ML_FN_READ_PROPERTY;

const char *
ml_graph_get_description (MlGraph *g)
ML_FN_READ_PROPERTY;

MlCaption *
ml_graph_get_caption (MlGraph *g)
ML_FN_READ_PROPERTY;

void
ml_graph_update_caption (MlGraph *g);

MlSurface *
ml_graph_get_surface (MlGraph *g)
ML_FN_READ_PROPERTY;

MlConfig *
ml_graph_get_ml_config (MlGraph *g)
ML_FN_READ_PROPERTY; // use ONLY for advanced reading (eg. type of keys, bounds, etc)! To read/write config, use ml_graph_set_config and ml_graph_set_config

const MlGraphStyle *
ml_graph_get_style (MlGraph *g)
ML_FN_READ_PROPERTY;

bool
ml_graph_set_style (MlGraph *g, MlGraphStyle *style)
ML_FN_WARN_UNUSED_RESULT;

uint32_t
ml_graph_get_ceiling (MlGraph *g)
ML_FN_READ_PROPERTY;

bool
ml_graph_set_ceiling (MlGraph *g, uint32_t ceiling)
ML_FN_WARN_UNUSED_RESULT;

int
ml_graph_get_border_size (MlGraph *g)
ML_FN_READ_PROPERTY;

bool
ml_graph_set_border_size (MlGraph *g, int border_size)
ML_FN_WARN_UNUSED_RESULT;

bool
ml_graph_set_config (MlGraph *g, const char *key, const char *value)
ML_FN_WARN_UNUSED_RESULT;

const char *
ml_graph_get_config (MlGraph *g, const char *key);

bool
ml_graph_collect_data (MlGraph *g)
ML_FN_WARN_UNUSED_RESULT ML_FN_HOT;

void
ml_graph_draw (MlGraph *g)
ML_FN_HOT;

cJSON *
ml_graph_to_json (MlGraph *g)
ML_FN_COLD;

void
ml_graph_dump_dataset (MlGraph *g)
ML_FN_COLD;


ML_HEADER_END
#endif /* ML_HEADER__GRAPH_GRAPH_H__INCLUDED */
