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


#ifndef __AUTOSCALER_H__
#define __AUTOSCALER_H__

#include <glib.h>
#include <time.h>

#include "multiload.h"

G_BEGIN_DECLS

/* Min value of the the maximum. Note that the autoscaler
 * is used to represent transfer rates (byte/sec) */
#define AUTOSCALER_FLOOR 500

typedef struct {
	unsigned max;
	unsigned count;
	time_t last_update;
	float sum;
	float last_average;
} AutoScaler;


G_GNUC_INTERNAL unsigned
autoscaler_get_max(AutoScaler *s, LoadGraph *g, unsigned current);

G_END_DECLS

#endif /* __AUTOSCALER_H__ */
