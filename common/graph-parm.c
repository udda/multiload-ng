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

#include <math.h>
#include <glib/gi18n-lib.h>

#include "graph-data.h"
#include "autoscaler.h"
#include "preferences.h"
#include "util.h"


void
multiload_graph_parm_get_data (int Maximum, int data[4], LoadGraph *g, ParametricData *xd, gboolean first_call)
{
	int max;
	gboolean spawn_success;

	gchar *stdout = NULL;
	gchar *stderr = NULL;
	int exit_status;

	guint i;
	gdouble total = 0;

	if (xd->command[0] == '\0') {
		xd->error = TRUE;
		snprintf(xd->message, sizeof(xd->message), _("Command line is empty."));
	} else if ((spawn_success= g_spawn_command_line_sync (xd->command, &stdout, &stderr, &exit_status, NULL)) == FALSE) {
		xd->error = TRUE;
		snprintf(xd->message, sizeof(xd->message), _("Unable to execute command."));
	} else if (exit_status != 0) {
		xd->error = TRUE;
		snprintf(xd->message, sizeof(xd->message), _("Command has exited with status code %d."), exit_status);
	} else {
		/* child process:
		 * - MUST 'exit 0'
		 * - MUST print to stdout from 1 up to 4 POSITIVE doubles, separated by spaces or newline.
		 *   They can be prefix with 0 (octal) or 0x (hex). Negative numbers will be set to 0.
		 *   Any additional content after the numbers is ignored. Point/comma depends on locale settings.
		 * - CAN print some text on stderr. Any additional content after the first line is ignored
		 */

		xd->nvalues = sscanf(stdout, "%lf %lf %lf %lf", xd->result+0, xd->result+1, xd->result+2, xd->result+3);
		if (xd->nvalues < 1) {
			xd->error = TRUE;
			snprintf(xd->message, sizeof(xd->message), _("Command did not return valid numbers."));
		} else {
			xd->error = FALSE;
			//copy first line of stderr to xd->message
			if (stderr != NULL) {
				guint i;
				for (i=0; i<sizeof(xd->message)-1; i++) {
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

	if (xd->error == TRUE) {
		memset(xd->result, 0, 4*sizeof(xd->result[0]));
	} else {
		for (i=0; i<4; i++) {
			if (xd->result[i] < 0)
				xd->result[i] = 0;
			else
				total += xd->result[i];
		}
	}

	if (g == NULL || data == NULL || Maximum == 0)
		return; // allow this function to be used just to test command lines

	max = autoscaler_get_max(&xd->scaler, g, rint(total));
	if (max == 0) {
		memset(data, 0, 4*sizeof(data[0]));
	} else {
		for (i=0; i<4; i++)
			data[i] = rint (Maximum * (float)xd->result[i] / max);
	}
}


void
multiload_graph_parm_cmdline_output (LoadGraph *g, ParametricData *xd)
{
	guint i;
	for (i=0; i<4; i++)
		g_snprintf(g->output_str[i], sizeof(g->output_str[i]), "%lf", xd->result[i]);
}


void
multiload_graph_parm_tooltip_update (char **title, char **text, LoadGraph *g, ParametricData *xd)
{
	if (g->config->tooltip_style == MULTILOAD_TOOLTIP_STYLE_DETAILED) {
		if (xd->error)
			*text = g_strdup_printf(_(	"Command: %s\n"
										"ERROR: %s"),
										xd->command, xd->message);
		else {
			if (xd->message[0] != '\0')
				*title = g_strdup(xd->message);
			*text = g_strdup_printf(_(	"Command: %s\n"
										"Results: (%.3lf, %.3lf, %.3lf, %.3lf)"),
										xd->command, xd->result[0], xd->result[1],
										xd->result[2], xd->result[3]);
		}
	} else {
		if (xd->error)
			*text = g_strdup_printf(_(	"ERROR: %s"), xd->message);
		else if (xd->message[0] != '\0')
			*text = g_strdup(xd->message);
		else
			*text = g_strdup_printf("(%lf, %lf, %lf, %lf)",
										xd->result[0], xd->result[1],
										xd->result[2], xd->result[3]);
	}
}
