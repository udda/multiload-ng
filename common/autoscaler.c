/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *               2002 The Free Software Foundation
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
#include <time.h>
#include <glib.h>

#include "autoscaler.h"
#include "graph-data.h"
#include "multiload-config.h"


int
autoscaler_get_max(AutoScaler *s, LoadGraph *g, int current)
{
	time_t now;
	if (current < 0)
		current = 0;

	if (s->min < 0)
		s->min = AUTOSCALER_MIN_DEFAULT;

	if (s->enable) {
		s->sum += current;
		s->count++;
		time(&now);
		if ((gdouble)difftime(now, s->last_update) > (g->draw_width * g->config->interval / 1000)) {
			gdouble new_average = s->sum / s->count;
			gdouble average;

			if (new_average < s->last_average)
				average = ((s->last_average * 0.5f) + new_average) / 1.5f;
			else
				average = new_average;

			s->max = average * 1.2f;

			s->sum = 0.0f;
			s->count = 0;
			s->last_update = now;
			s->last_average = average;
			g_debug("[autoscaler] Recalculated max for graph '%s': %d", graph_types[g->id].name, s->max);
		}

		s->max = MAX(s->max, current);
		s->max = MAX(s->max, s->min);
	}

	return s->max;
}

void
autoscaler_set_max(AutoScaler *s, int max)
{
	if (s->enable == FALSE)
		s->max = max;
}

void
autoscaler_set_min(AutoScaler *s, int min)
{
	s->min = min;
}

void
autoscaler_set_enabled(AutoScaler *s, gboolean enable)
{
	s->enable = enable;
}

gboolean
autoscaler_get_enabled(AutoScaler *s)
{
	return s->enable;
}


AutoScaler*
multiload_get_scaler (MultiloadPlugin *ma, guint graph_id)
{
	gpointer xd = ma->extra_data[graph_id];

	switch(graph_id) {
		case GRAPH_CPULOAD:
		case GRAPH_MEMLOAD:
		case GRAPH_SWAPLOAD:
			// no autoscaler
			return NULL;
		case GRAPH_NETLOAD:
			return &((NetData*)xd)->scaler;
		case GRAPH_LOADAVG:
			return &((LoadData*)xd)->scaler;
		case GRAPH_DISKLOAD:
			return &((DiskData*)xd)->scaler;
		case GRAPH_TEMPERATURE:
			return &((TemperatureData*)xd)->scaler;
		case GRAPH_PARAMETRIC:
			return &((ParametricData*)xd)->scaler;
		default:
			g_assert_not_reached();
	}
}
