/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of multiload-nandhp.
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

#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include <gtk/gtk.h>

#include "multiload.h"


G_BEGIN_DECLS

/* DEFINITIONS OF PROPERTIES AND ACTIONS */

// properties and actions are all in the first 16 bits; this make room for another 16 bit value
enum MultiloadProperties {
	PROP_SHOWGRAPH			= 0x00010000,
	PROP_SPEED				= 0x00020000,
	PROP_SIZE				= 0x00030000,
	PROP_PADDING			= 0x00040000,
	PROP_SPACING			= 0x00050000,
	PROP_ORIENTATION		= 0x00060000,
	PROP_BORDERWIDTH		= 0x00070000,
	PROP_COLOR				= 0x00080000,
	PROP_FILLBETWEEN		= 0x00090000,
	PROP_TOOLTIP_DETAILS	= 0x000A0000,
	PROP_DBLCLICK_POLICY	= 0x000B0000,
	PROP_DBLCLICK_CMDLINE	= 0x000C0000
};

enum MultiloadActions {
	ACTION_DEFAULT_COLORS	= 0x01000000,
	ACTION_EXPORT_COLORS	= 0x02000000,
	ACTION_IMPORT_COLORS	= 0x03000000
};




/* PROPERTIES VALUES */

enum MultiloadOrientationPolicy {
	MULTILOAD_ORIENTATION_AUTO			= 0,
	MULTILOAD_ORIENTATION_HORIZONTAL	= 1,
	MULTILOAD_ORIENTATION_VERTICAL		= 2,

	MULTILOAD_ORIENTATION_N_VALUES		= 2
};

enum MultiloadDblClickPolicy {
	DBLCLICK_POLICY_DONOTHING			= 0,
	DBLCLICK_POLICY_TASKMANAGER			= 1,
	DBLCLICK_POLICY_CMDLINE				= 2,

	DBLCLICK_POLICY_N_VALUES			= 2
};

enum MultiloadLimits {
	MIN_SIZE							= 10,
	MAX_SIZE							= 400,

	MIN_SPEED							= 50,
	MAX_SPEED							= 20000,

	MIN_PADDING							= 0,
	MAX_PADDING							= 40,

	MIN_SPACING							= 0,
	MAX_SPACING							= 40,

	MIN_BORDER_WIDTH					= 0,
	MAX_BORDER_WIDTH					= 16
};

enum MultiloadStep {
	STEP_SIZE							= 5,
	STEP_SPEED							= 50,
	STEP_PADDING						= 1,
	STEP_SPACING						= 1,
	STEP_BORDER_WIDTH					= 1
};

enum MultiloadDefaults {
	DEFAULT_SIZE						= 40,
	DEFAULT_SPEED						= 500,
	DEFAULT_PADDING						= 2,
	DEFAULT_SPACING						= 1,
	DEFAULT_BORDER_WIDTH				= 1,
	DEFAULT_ORIENTATION					= MULTILOAD_ORIENTATION_AUTO,
	DEFAULT_DBLCLICK_POLICY				= DBLCLICK_POLICY_DONOTHING,
	DEFAULT_FILL_BETWEEN				= FALSE,
	DEFAULT_TOOLTIP_DETAILS				= FALSE
};


G_GNUC_INTERNAL void
multiload_init_preferences(GtkWidget *dialog, MultiloadPlugin *ma);

G_END_DECLS

#endif /* __PROPERTIES_H__ */
