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
#include "multiload-config.h"


guint64
autoscaler_get_max(AutoScaler *s, LoadGraph *g, guint64 current)
{
	time_t now;

	//TEMPORARY: autoscaler always enabled
	s->enable = TRUE;

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
			g_debug("[autoscaler] Recalculated max for graph '%s': %ld", graph_types[g->id].name, s->max);
		}

		s->max = MAX(s->max, current);
		s->max = MAX(s->max, AUTOSCALER_FLOOR);
	}

	return s->max;
}

void
autoscaler_set_max(AutoScaler *s, guint64 max)
{
	if (s->enable == FALSE)
		s->max = max;
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
