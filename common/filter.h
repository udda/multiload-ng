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


#ifndef __MULTILOAD_FILTER_H__
#define __MULTILOAD_FILTER_H__

#include <glib.h>


G_BEGIN_DECLS

typedef enum {
	MULTILOAD_FILTER_STATE_UNSELECTED,
	MULTILOAD_FILTER_STATE_SELECTED,
	MULTILOAD_FILTER_STATE_ABSENT
} MultiloadFilterElementState;

typedef struct {
	char data[512];
	char label[30];
	MultiloadFilterElementState state;
} MultiloadFilterElement;

typedef struct {
	GArray *array;
	guint length;
} MultiloadFilter;


G_GNUC_INTERNAL MultiloadFilter*
multiload_filter_new();

G_GNUC_INTERNAL MultiloadFilter*
multiload_filter_new_from_existing(gchar *existing);

G_GNUC_INTERNAL guint
multiload_filter_get_length(MultiloadFilter *filter);

G_GNUC_INTERNAL guint
multiload_filter_append(MultiloadFilter *filter, char *data);

G_GNUC_INTERNAL guint
multiload_filter_append_with_label(MultiloadFilter *filter, char *data, char *label);

G_GNUC_INTERNAL guint
multiload_filter_append_with_state(MultiloadFilter *filter, char *data, MultiloadFilterElementState state);

G_GNUC_INTERNAL void
multiload_filter_import_existing(MultiloadFilter *filter, char *existing);

void
multiload_filter_export(MultiloadFilter *filter, char *buf, size_t len);

G_GNUC_INTERNAL MultiloadFilterElement*
multiload_filter_get_element(MultiloadFilter *filter, guint index);

G_GNUC_INTERNAL gchar*
multiload_filter_get_element_data(MultiloadFilter *filter, guint index);
G_GNUC_INTERNAL gchar*
multiload_filter_get_element_label(MultiloadFilter *filter, guint index);
G_GNUC_INTERNAL gboolean
multiload_filter_get_element_selected(MultiloadFilter *filter, guint index);
G_GNUC_INTERNAL gboolean
multiload_filter_get_element_absent(MultiloadFilter *filter, guint index);

G_GNUC_INTERNAL void
multiload_filter_free(MultiloadFilter* filter);

G_END_DECLS

#endif /* __MULTILOAD_FILTER_H__ */
