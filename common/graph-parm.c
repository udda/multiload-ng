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


#include <config.h>

#include <errno.h>
#include <math.h>
#include <glib/gi18n-lib.h>

#include "graph-data.h"
#include "autoscaler.h"
#include "preferences.h"
#include "util.h"


void
multiload_graph_parm_get_data (int Maximum, int data[1], LoadGraph *g, ParametricData *xd)
{
	int max;
	gboolean spawn_success;

	gchar *stdout = NULL;
	gchar *stderr = NULL;
	int exit_status;

	if (xd->command[0] == '\0') {
		xd->error = TRUE;
		snprintf(xd->message, sizeof(xd->message), _("Command line is empty."));
	}

	spawn_success= g_spawn_command_line_sync (xd->command, &stdout, &stderr, &exit_status, NULL);
	if (!spawn_success) {
		xd->error = TRUE;
		xd->result = 0;
		snprintf(xd->message, sizeof(xd->message), _("Unable to execute command."));
	} else if (exit_status != 0) {
		xd->error = TRUE;
		xd->result = 0;
		snprintf(xd->message, sizeof(xd->message), _("Command has exited with status code %d."), exit_status);
	} else {
		/* child process:
		 * - MUST 'exit 0'
		 * - MUST print a single POSITIVE number on stdout, or at least output must start with a number - unsigned 64 bit - can prefix with 0 (octal) or 0x (hex)
		 * - CAN print some text on stderr, only the first line will be displayed
		 */

		errno = 0;
		xd->result = g_ascii_strtoull (stdout, NULL, 0);
		if (errno != 0) {
			xd->error = TRUE;
			snprintf(xd->message, sizeof(xd->message), _("Command did not return a valid number."));
		} else {
			xd->error = FALSE;
			//copy first line of stderr to xd->message
			if (stderr != NULL) {
				guint i;
				for (i=0; i<sizeof(xd->message); i++) {
					if (stderr[i] == '\n' || stderr[i] == '\r')
						stderr[i] = '\0';

					xd->message[i] = stderr[i];

					if (stderr[i] == '\0')
						break;
				}
				xd->message[sizeof(xd->message)-1] = '\0';
			} else
				xd->message[0] = '\0';
		}
	}

	max = autoscaler_get_max(&xd->scaler, g, xd->result);
	data[0] = rint (Maximum * (float)xd->result / max);
}

void
multiload_graph_parm_tooltip_update (char **title, char **text, LoadGraph *g, ParametricData *xd)
{
	if (g->config->tooltip_style == TOOLTIP_STYLE_DETAILS) {
		if (xd->error)
			*text = g_strdup_printf(_(	"Command: %s\n"
										"ERROR: %s"),
										xd->command, xd->message);
		else
			*text = g_strdup_printf(_(	"Command: %s\n"
										"Result: %lu\n"
										"Message: %s"),
										xd->command, xd->result, xd->message);
	} else {
		*text = g_strdup_printf("%lu", xd->result);
	}
}
