/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
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


#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include <gtk/gtk.h>

#include "multiload.h"


G_BEGIN_DECLS


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

enum MultiloadTooltipStyle {
	TOOLTIP_STYLE_SIMPLE				= 0,
	TOOLTIP_STYLE_DETAILS				= 1,

	TOOLTIP_STYLE_N_VALUES				= 1
};


enum MultiloadLimits {
	MIN_SIZE							= 10,
	MAX_SIZE							= 400,

	MIN_INTERVAL						= 50,
	MAX_INTERVAL						= 20000,

	MIN_PADDING							= 0,
	MAX_PADDING							= 40,

	MIN_SPACING							= 0,
	MAX_SPACING							= 40,

	MIN_BORDER_WIDTH					= 0,
	MAX_BORDER_WIDTH					= 16
};

enum MultiloadStep {
	STEP_SIZE							= 5,
	STEP_INTERVAL						= 50,
	STEP_PADDING						= 1,
	STEP_SPACING						= 1,
	STEP_BORDER_WIDTH					= 1
};

enum MultiloadDefaults {
	DEFAULT_SIZE						= 40,
	DEFAULT_INTERVAL					= 500,
	DEFAULT_PADDING						= 2,
	DEFAULT_SPACING						= 1,
	DEFAULT_BORDER_WIDTH				= 1,
	DEFAULT_ORIENTATION					= MULTILOAD_ORIENTATION_AUTO,
	DEFAULT_DBLCLICK_POLICY				= DBLCLICK_POLICY_DONOTHING,
	DEFAULT_FILL_BETWEEN				= FALSE,
	DEFAULT_TOOLTIP_STYLE				= TOOLTIP_STYLE_SIMPLE,

	DEFAULT_MAX_VALUE					= -1,
	DEFAULT_MAX_VALUE_LOAD				= 8,		// should cover well single to quad cores
	DEFAULT_MAX_VALUE_TEMP				= 120000	// measured in millidegrees
};
#define DEFAULT_COLOR_SCHEME "Multiload-ng"


G_GNUC_INTERNAL void
multiload_preferences_update_color_buttons(MultiloadPlugin *ma);

G_GNUC_INTERNAL void
multiload_preferences_fill_dialog(GtkWidget *dialog, MultiloadPlugin *ma);

G_END_DECLS

#endif /* __PROPERTIES_H__ */
