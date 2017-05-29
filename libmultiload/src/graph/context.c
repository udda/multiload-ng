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


struct MlGraphContext_ {
	const MlGraphTypeInterface *iface;
	mlPointer provider_data;

	int n_data;
	uint32_t *data;
	uint32_t *max;

	bool first_call;
	bool need_data_reset;

	int assert_no;
	bool fail;
	char *fail_msg;
};


MlGraphContext*
ml_graph_context_new (const MlGraphTypeInterface *iface, MlConfig *config)
{
	if_unlikely (iface == NULL || config == NULL)
		return NULL;

	if (iface->n_data < 1)
		return NULL;

	MlGraphContext *context = ml_new (MlGraphContext);

	context->iface = iface;
	context->n_data = iface->n_data;
	context->first_call = true;
	if (iface->dataset_mode == ML_DATASET_MODE_PROPORTIONAL || iface->dataset_mode == ML_DATASET_MODE_ABSOLUTE) { // need max
		context->data = ml_new_n (uint32_t, iface->n_data+1);
		context->max = context->data + iface->n_data;
	} else {
		context->data = ml_new_n (uint32_t, iface->n_data);
		context->max = NULL;
	}

	if (iface->init_fn != NULL)
		context->provider_data = iface->init_fn (config);

	return context;
}

void
ml_graph_context_destroy (MlGraphContext *context)
{
	if_unlikely (context == NULL)
		return;

	if (context->iface->destroy_fn != NULL)
		context->iface->destroy_fn (context->provider_data);

	free (context->fail_msg);
	free (context->data);
	free (context);
}

size_t
ml_graph_context_sizeof (MlGraphContext *context)
{
	if_unlikely (context == NULL)
		return 0;

	size_t size = sizeof (MlGraphContext);

	size += sizeof (uint32_t) * context->iface->n_data; // data

	if (context->max != NULL)
		size += sizeof (uint32_t);

	size += ml_string_sizeof (context->fail_msg);

	if (context->iface->sizeof_fn != NULL)
		size += context->iface->sizeof_fn (context->provider_data);

	return size;
}

void
ml_graph_context_before (MlGraphContext *context)
{
	// call that BEFORE using the context (eg. graph's get_data() or config() functions)
	if_unlikely (context == NULL)
		return;

	memset (context->data, 0, sizeof(uint32_t) * context->n_data);
	if (context->max != NULL)
		*context->max = 0;

	context->assert_no = 0;
	context->fail = false;
	context->need_data_reset = false;

	if (context->fail_msg != NULL) {
		free (context->fail_msg);
		context->fail_msg = NULL;
	}
}

void
ml_graph_context_after (MlGraphContext *context)
{
	// call that AFTER using the context (eg. graph's get_data() or config() functions)
	if_unlikely (context == NULL)
		return;

	if (!context->need_data_reset)
		context->first_call = false;
}

/* Use this function to check a condition that causes an error. Every time this function
 * is called, an internal counter is incremented, so the exact failure point can be
 * retrieved later. */
bool
ml_graph_context_check_condition (MlGraphContext *context, bool condition, const char *errormsg_fmt, ...)
{
	if_unlikely (context == NULL)
		return condition;

	// maintain first error code
	if (context->fail)
		return condition;

	context->assert_no ++;
	if_unlikely (!condition) {
		context->fail = true;
		if (errormsg_fmt != NULL) {
			free (context->fail_msg);

			va_list va;
			va_start (va, errormsg_fmt);
			context->fail_msg = ml_strdup_vprintf (errormsg_fmt, va);
			va_end (va);
		}
	}

	return condition;
}

void
ml_graph_context_set_first_call (MlGraphContext *context)
{
	if_likely (context != NULL)
		context->first_call = true;
}

bool
ml_graph_context_is_first_call (MlGraphContext *context)
{
	if_unlikely (context == NULL)
		return true;

	return context->first_call;
}

const char *
ml_graph_context_get_iface_name (MlGraphContext *context)
{
	if_unlikely (context == NULL || context->iface == NULL)
		return NULL;

	return (const char*)context->iface->name;
}

void
ml_graph_context_set_need_data_reset (MlGraphContext *context)
{
	if_unlikely (context == NULL)
		return;

	context->first_call = true;
	context->need_data_reset = true;
}

bool
ml_graph_context_get_need_data_reset (MlGraphContext *context)
{
	if_unlikely (context == NULL)
		return true;

	return context->need_data_reset;
}

int
ml_graph_context_get_n_data (MlGraphContext *context)
{
	// return the data size (excluding max when present), useful for providers
	if_unlikely (context == NULL)
		return 0;

	return context->n_data;
}

int
ml_graph_context_get_fail_code (MlGraphContext *context)
{
	// return 0 on success, nonzero on failure (code is incremented on every ml_graph_context_assert)
	if_unlikely (context == NULL)
		return 0;

	if (!context->fail)
		return 0;

	return context->assert_no;
}

const char *
ml_graph_context_get_fail_message (MlGraphContext *context)
{
	// return NULL on success, fail message (if specified) on failure
	if_unlikely (context == NULL)
		return 0;

	if (!context->fail)
		return 0;

	return context->fail_msg;
}

mlPointer
ml_graph_context_get_provider_data (MlGraphContext *context)
{
	if_unlikely (context == NULL)
		return NULL;

	return context->provider_data;
}

void
ml_graph_context_set_data (MlGraphContext *context, int index, uint32_t value)
{
	if_unlikely (context == NULL || context->data == NULL)
		return;

	if (index < 0 || index >= context->n_data)
		return;

	context->data[index] = value;
}

void
ml_graph_context_set_max (MlGraphContext *context, uint32_t max)
{
	if_unlikely (context == NULL || context->max == NULL)
		return;

	*(context->max) = max;
}

void
ml_graph_context_push_to_dataset (MlGraphContext *context, MlDataset *ds)
{
	if_unlikely (context == NULL || context->iface == NULL)
		return;

	int n_data_real;
	if (context->iface->dataset_mode == ML_DATASET_MODE_PROPORTIONAL || context->iface->dataset_mode == ML_DATASET_MODE_ABSOLUTE)
		n_data_real = context->n_data + 1;
	else
		n_data_real = context->n_data;

	ml_dataset_push_entry (ds, context->data, n_data_real);
}
