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


struct _MlCaption {
	MlGrowBuffer *header;
	MlGrowBuffer *body;
	MlGrowBuffer *footer;

	MlGrowBuffer *table_data [ML_CAPTION_TABLE_ROWS][ML_CAPTION_TABLE_COLUMNS];
};


MlCaption *
ml_caption_new ()
{
	MlCaption *cap = ml_new (MlCaption);

	cap->header = ml_grow_buffer_new (0);
	cap->body = ml_grow_buffer_new (0);
	cap->footer = ml_grow_buffer_new (0);

	for (int col = 0; col < ML_CAPTION_TABLE_COLUMNS; col++)
		for (int row = 0; row < ML_CAPTION_TABLE_ROWS; row++)
			cap->table_data[row][col] = ml_grow_buffer_new (0);

	return cap;
}

void
ml_caption_destroy (MlCaption *cap)
{
	if_unlikely (cap == NULL)
		return;

	ml_grow_buffer_destroy (cap->header, true);
	ml_grow_buffer_destroy (cap->body, true);
	ml_grow_buffer_destroy (cap->footer, true);

	for (int col = 0; col < ML_CAPTION_TABLE_COLUMNS; col++)
		for (int row = 0; row < ML_CAPTION_TABLE_ROWS; row++)
			ml_grow_buffer_destroy (cap->table_data[row][col], true);

	free (cap);
}

size_t
ml_caption_sizeof (MlCaption *cap)
{
	if_unlikely (cap == NULL)
		return 0;

	return sizeof (MlCaption);
}

void
ml_caption_clear (MlCaption *cap)
{
	if_unlikely (cap == NULL)
		return;

	ml_grow_buffer_rewind (cap->header);
	ml_grow_buffer_rewind (cap->body);
	ml_grow_buffer_rewind (cap->footer);

	for (int col = 0; col < ML_CAPTION_TABLE_COLUMNS; col++)
		for (int row = 0; row < ML_CAPTION_TABLE_ROWS; row++)
			ml_grow_buffer_rewind (cap->table_data[row][col]);
}

static void
_ml_caption_print_impl (MlCaption *cap, MlCaptionComponent component, const char *fmt, bool append, va_list va)
{
	if_unlikely (cap == NULL || fmt == NULL)
		return;

	switch (component) {
		case ML_CAPTION_COMPONENT_HEADER:
			if (!append)
				ml_grow_buffer_rewind (cap->header);
			ml_grow_buffer_append_vprintf (cap->header, fmt, va);
			break;
		case ML_CAPTION_COMPONENT_BODY:
			if (!append)
				ml_grow_buffer_rewind (cap->body);
			ml_grow_buffer_append_vprintf (cap->body, fmt, va);
			break;
		case ML_CAPTION_COMPONENT_FOOTER:
			if (!append)
				ml_grow_buffer_rewind (cap->footer);
			ml_grow_buffer_append_vprintf (cap->footer, fmt, va);
			break;
		default:
			ml_error ("Unknown component (%d)", component);
			break;
	}
}


void
ml_caption_set (MlCaption *cap, MlCaptionComponent component, const char *fmt, ...)
{
	if_unlikely (cap == NULL || fmt == NULL)
		return;

	va_list va;
	va_start (va, fmt);

	_ml_caption_print_impl (cap, component, fmt, false, va);

	va_end (va);
}

void
ml_caption_append (MlCaption *cap, MlCaptionComponent component, const char *fmt, ...)
{
	if_unlikely (cap == NULL || fmt == NULL)
		return;

	va_list va;
	va_start (va, fmt);

	_ml_caption_print_impl (cap, component, fmt, true, va);

	va_end (va);
}

static void
_ml_caption_table_print_impl (MlCaption *cap, int row, int col, const char *fmt, bool append, va_list va)
{
	if_unlikely (cap == NULL || fmt == NULL)
		return;

	if (row < 0 || row >= ML_CAPTION_TABLE_ROWS || col < 0 || col >= ML_CAPTION_TABLE_COLUMNS) {
		ml_error ("Table coordinates out of bounds (%d, %d)", row, col);
		return;
	}

	if (!append)
		ml_grow_buffer_rewind (cap->table_data[row][col]);
	ml_grow_buffer_append_vprintf (cap->table_data[row][col], fmt, va);
}

void
ml_caption_set_table_data (MlCaption *cap, int row, int col, const char *fmt, ...)
{
	if_unlikely (cap == NULL || fmt == NULL)
		return;

	va_list va;
	va_start (va, fmt);

	_ml_caption_table_print_impl (cap, row, col, fmt, false, va);

	va_end (va);
}

void
ml_caption_append_table_data (MlCaption *cap, int row, int col, const char *fmt, ...)
{
	if_unlikely (cap == NULL || fmt == NULL)
		return;

	va_list va;
	va_start (va, fmt);

	_ml_caption_table_print_impl (cap, row, col, fmt, true, va);

	va_end (va);
}

const char *
ml_caption_get (MlCaption *cap, MlCaptionComponent component)
{
	if_unlikely (cap == NULL)
		return NULL;

	switch (component) {
		case ML_CAPTION_COMPONENT_HEADER:
			return ml_grow_buffer_get_string (cap->header);
		case ML_CAPTION_COMPONENT_BODY:
			return ml_grow_buffer_get_string (cap->body);
		case ML_CAPTION_COMPONENT_FOOTER:
			return ml_grow_buffer_get_string (cap->footer);
		default:
			ml_error ("Unknown component (%d)", component);
			return NULL;
	}
}

const char *
ml_caption_get_table_data (MlCaption *cap, int row, int col)
{
	if_unlikely (cap == NULL)
		return NULL;

	if (row < 0 || row >= ML_CAPTION_TABLE_ROWS || col < 0 || col >= ML_CAPTION_TABLE_COLUMNS) {
		ml_error ("Table coordinates out of bounds (%d, %d)", row, col);
		return NULL;
	}

	return ml_grow_buffer_get_string (cap->table_data[row][col]);
}

cJSON *
ml_caption_to_json (MlCaption *cap)
{
	if_unlikely (cap == NULL)
		return NULL;

	cJSON* obj = cJSON_CreateObject ();

	cJSON_AddStringToObject (obj, "header", ml_grow_buffer_get_string (cap->header));
	cJSON_AddStringToObject (obj, "body", ml_grow_buffer_get_string (cap->body));
	cJSON_AddStringToObject (obj, "footer", ml_grow_buffer_get_string (cap->footer));

	cJSON *obj_table = cJSON_CreateArray ();

	for (int row = 0; row < ML_CAPTION_TABLE_ROWS; row++) {
		cJSON *obj_row = cJSON_CreateArray ();

		for (int col = 0; col < ML_CAPTION_TABLE_COLUMNS; col++) {
			cJSON_AddItemToArray (obj_row, cJSON_CreateString(ml_grow_buffer_get_string (cap->table_data[row][col])));
		}

		cJSON_AddItemToArray (obj_table, obj_row);
	}

	cJSON_AddItemToObject (obj, "table", obj_table);
	return obj;
}

