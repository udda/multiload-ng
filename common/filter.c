/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *                    The Free Software Foundation
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


#include <config.h>
#include <string.h>

#include "filter.h"

/* Requirements for filter separators:
 * 1. Must characters/sequences that never appear in filter elements.
 *    By now, filter elements are all files in /sys or /dev.
 * 2. To play safe, longer sequences can be used.
 * 3. Must not contain newlines or invalid UTF-8 sequences. */
#define MULTILOAD_FILTER_SEPARATOR "|"


MultiloadFilter*
multiload_filter_new()
{
	MultiloadFilter *filter = g_new0(MultiloadFilter, 1);

	filter->array = g_array_sized_new (TRUE, TRUE, sizeof(MultiloadFilterElement), 10);
	filter->length = 0;

	return filter;
}

MultiloadFilter*
multiload_filter_new_from_existing(gchar *existing)
{
	guint i;
	MultiloadFilter *filter = multiload_filter_new();
	gchar **split = g_strsplit(existing, MULTILOAD_FILTER_SEPARATOR, -1);

	for (i=0; split[i]!=NULL; i++) {
		multiload_filter_append(filter, split[i]);
	}

	g_strfreev(split);

	return filter;
}

guint
multiload_filter_get_length(MultiloadFilter *filter)
{
	g_assert_nonnull(filter);

	return filter->length;
}

guint
multiload_filter_append(MultiloadFilter *filter, char *data)
{
	g_assert_nonnull(filter);

	MultiloadFilterElement element;
	memset(&element, 0, sizeof(element));

	strncpy(element.data, data, sizeof(element.data));
	element.state = MULTILOAD_FILTER_STATE_UNSELECTED;

	g_array_append_val(filter->array, element);
	filter->length++;

	return filter->length;
}

guint
multiload_filter_append_with_label(MultiloadFilter *filter, char *data, char *label)
{
	g_assert_nonnull(filter);

	guint n = multiload_filter_append(filter, data);

	MultiloadFilterElement *element = multiload_filter_get_element(filter, n);
	strncpy(element->label, label, sizeof(element->label));
	return n;
}

guint
multiload_filter_append_with_state(MultiloadFilter *filter, char *data, MultiloadFilterElementState state)
{
	g_assert_nonnull(filter);

	guint n = multiload_filter_append(filter, data);

	MultiloadFilterElement *element = multiload_filter_get_element(filter, n);
	element->state = state;
	return n;
}

void
multiload_filter_import_existing(MultiloadFilter *filter, char *existing)
{
	g_assert_nonnull(filter);

	guint i, j;
	MultiloadFilterElement *e;

	char **split = g_strsplit(existing, MULTILOAD_FILTER_SEPARATOR, -1);

	// set selected status on already present elements, and removes them from **split
	for (i=0; split[i] != NULL; i++) {
		for (j=0; j<filter->length; j++) {
			e = multiload_filter_get_element(filter,j);
			if (strcmp(split[i], e->data) == 0) {
				e->state = MULTILOAD_FILTER_STATE_SELECTED;
				split[i][0] = '\0';
				break;
			}
		}
	}

	// add remaining elements
	for (i=0; split[i] != NULL; i++) {
		if (split[i][0] == '\0')
			continue;

		j = multiload_filter_append(filter, split[i]);
		e = multiload_filter_get_element(filter, j-1);
		e->state = MULTILOAD_FILTER_STATE_ABSENT;
	}

	g_strfreev(split);
}

void
multiload_filter_export(MultiloadFilter *filter, char *buf, size_t len)
{
	g_assert_nonnull(filter);

	guint i;
	char *data;

	buf[0] = '\0';
	for (i=0; i<multiload_filter_get_length(filter); i++) {
		data = multiload_filter_get_element_data(filter, i);

		// element data must not contain filter separator - if it does, separator must be changed!
		g_assert_null( strstr(data, MULTILOAD_FILTER_SEPARATOR) );

		g_strlcat(buf, data, len);
		g_strlcat(buf, MULTILOAD_FILTER_SEPARATOR, len);
	}
}

MultiloadFilterElement*
multiload_filter_get_element(MultiloadFilter *filter, guint index)
{
	g_assert_nonnull(filter);

	// do not free returned pointer!
	return &g_array_index (filter->array, MultiloadFilterElement, index);
}

gchar*
multiload_filter_get_element_label(MultiloadFilter *filter, guint index)
{
	// do not free returned pointer!
	MultiloadFilterElement *e = multiload_filter_get_element(filter, index);
	if (e->label[0] != '\0')
		return e->label;
	else
		return e->data;
}

gchar*
multiload_filter_get_element_data(MultiloadFilter *filter, guint index)
{
	// do not free returned pointer!
	MultiloadFilterElement *e = multiload_filter_get_element(filter, index);
	return e->data;
}

gboolean
multiload_filter_get_element_selected(MultiloadFilter *filter, guint index)
{
	// do not free returned pointer!
	MultiloadFilterElement *e = multiload_filter_get_element(filter, index);
	if (e->state & MULTILOAD_FILTER_STATE_SELECTED)
		return TRUE;
	if (e->state & MULTILOAD_FILTER_STATE_ABSENT)
		return TRUE;
	return FALSE;
}

gboolean
multiload_filter_get_element_absent(MultiloadFilter *filter, guint index)
{
	// do not free returned pointer!
	MultiloadFilterElement *e = multiload_filter_get_element(filter, index);
	if (e->state & MULTILOAD_FILTER_STATE_ABSENT)
		return TRUE;
	return FALSE;
}

void
multiload_filter_free(MultiloadFilter* filter)
{
	if (filter == NULL)
		return;
	g_array_free(filter->array, TRUE);
	g_free (filter);
}