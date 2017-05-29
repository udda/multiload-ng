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

#ifndef ML_HEADER__GRAPH_CONTEXT_H__INCLUDED
#define ML_HEADER__GRAPH_CONTEXT_H__INCLUDED
ML_HEADER_BEGIN


typedef struct MlGraphContext_ MlGraphContext;
#include "type.h"


#define ml_graph_context_assert(context, condition) if_unlikely (!ml_graph_context_check_condition ((context), (condition), NULL)) { return; }
#define ml_graph_context_assert_with_message(context, condition, ...) if_unlikely (!ml_graph_context_check_condition ((context), (condition), __VA_ARGS__)) { return; }


MlGraphContext *
ml_graph_context_new (const MlGraphTypeInterface *iface, MlConfig *config);

void
ml_graph_context_destroy (MlGraphContext *context);

size_t
ml_graph_context_sizeof (MlGraphContext *context)
ML_FN_SIZEOF;

void
ml_graph_context_before (MlGraphContext *context)
ML_FN_HOT;

void
ml_graph_context_after (MlGraphContext *context)
ML_FN_HOT;

bool
ml_graph_context_check_condition (MlGraphContext *context, bool condition, const char *errormsg_fmt, ...)
ML_FN_PRINTF(3,4) ML_FN_WARN_UNUSED_RESULT;

void
ml_graph_context_set_first_call (MlGraphContext *context);

bool
ml_graph_context_is_first_call (MlGraphContext *context)
ML_FN_READ_PROPERTY;

const char *
ml_graph_context_get_iface_name (MlGraphContext *context)
ML_FN_READ_PROPERTY;

void
ml_graph_context_set_need_data_reset (MlGraphContext *context);

bool
ml_graph_context_get_need_data_reset (MlGraphContext *context)
ML_FN_READ_PROPERTY;

int
ml_graph_context_get_n_data (MlGraphContext *context)
ML_FN_READ_PROPERTY;

int
ml_graph_context_get_fail_code (MlGraphContext *context)
ML_FN_READ_PROPERTY;

const char *
ml_graph_context_get_fail_message (MlGraphContext *context)
ML_FN_READ_PROPERTY;

mlPointer
ml_graph_context_get_provider_data (MlGraphContext *context)
ML_FN_READ_PROPERTY;

void
ml_graph_context_set_data (MlGraphContext *context, int index, uint32_t value)
ML_FN_HOT;

void
ml_graph_context_set_max (MlGraphContext *context, uint32_t max)
ML_FN_HOT;

void
ml_graph_context_push_to_dataset (MlGraphContext *context, MlDataset *ds)
ML_FN_HOT;


ML_HEADER_END
#endif /* ML_HEADER__GRAPH_CONTEXT_H__INCLUDED */
