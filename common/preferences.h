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
	MULTILOAD_TOOLTIP_STYLE_SIMPLE		= 0,
	MULTILOAD_TOOLTIP_STYLE_DETAILED	= 1,

	MULTILOAD_TOOLTIP_STYLE_N_VALUES	= 1
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

enum MultiloadDefaults {
	DEFAULT_SIZE						= 40,
	DEFAULT_INTERVAL					= 1000,
	DEFAULT_PADDING						= 2,
	DEFAULT_SPACING						= 1,
	DEFAULT_BORDER_WIDTH				= 1,
	DEFAULT_ORIENTATION					= MULTILOAD_ORIENTATION_AUTO,
	DEFAULT_DBLCLICK_POLICY				= DBLCLICK_POLICY_DONOTHING,
	DEFAULT_FILL_BETWEEN				= FALSE,
	DEFAULT_TOOLTIP_STYLE				= MULTILOAD_TOOLTIP_STYLE_SIMPLE,
	DEFAULT_BACKGROUND_DIRECTION		= MULTILOAD_GRADIENT_LINEAR_N_TO_S,
	DEFAULT_SIZE_FORMAT_IEC				= TRUE
};

enum MultiloadSettingsType {
	MULTILOAD_SETTINGS_SIZE				= 1 << 0,
	MULTILOAD_SETTINGS_PADDING			= 1 << 1,
	MULTILOAD_SETTINGS_SPACING			= 1 << 2,
	MULTILOAD_SETTINGS_ORIENTATION		= 1 << 3,
	MULTILOAD_SETTINGS_FILL_BETWEEN		= 1 << 4,
	MULTILOAD_SETTINGS_TOOLTIPS			= 1 << 5,
	MULTILOAD_SETTINGS_TIMESPAN			= 1 << 6,
	MULTILOAD_SETTINGS_ORIENT_WARNING	= 1 << 7
};

#define DEFAULT_COLOR_SCHEME "Multiload-ng"


G_GNUC_INTERNAL void
multiload_preferences_update_color_buttons(MultiloadPlugin *ma);

G_GNUC_INTERNAL void
multiload_preferences_fill_dialog(GtkWidget *dialog, MultiloadPlugin *ma);

G_GNUC_INTERNAL void
multiload_preferences_disable_settings(guint mask);

G_END_DECLS

#endif /* __PROPERTIES_H__ */
