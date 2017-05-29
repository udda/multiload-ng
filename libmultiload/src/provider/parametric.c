/*
 * Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of Multiload-ng.
 * 
 * Multiload-ng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Multiload-ng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <multiload.h>


const MlGraphTypeInterface ML_PROVIDER_PARAMETRIC_IFACE = {
	.name			= "parametric",
	.label			= N_("Parametric"),
	.description	= N_("Customizable output using an external application."),

	.hue			= 293,

	.n_data			= 4,
	.dataset_mode	= ML_DATASET_MODE_ABSOLUTE,

	.init_fn		= ml_provider_parametric_init,
	.config_fn		= NULL,
	.get_fn			= ml_provider_parametric_get,
	.destroy_fn		= ml_provider_parametric_destroy,
	.sizeof_fn		= ml_provider_parametric_sizeof,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_parametric_caption,

	.helptext		= N_(
		"Selected process:"
		"<ul>"
			"<li>MUST exit with status code '0'</li>"
			"<li>MUST print to stdout 1 up to 4 numbers, separated by spaces or newline</li>"
				"Numbers can be integer or floating point. Floating point numbers will be read according to current locale <br />"
				"Numbers can begin with 0 (octal) or 0x (hex) <br />"
				"Numbers must not be negative. Negative numbers will be read as 0 <br />"
				"Text after the numbers is ignored"
			"</li>"
			"<li>CAN print some text on stderr. Any additional content after the first line is ignored</li>"
		"</ul>"
	)	
};


typedef struct {
	MlGrowBuffer *gbuf_stdout;
	MlGrowBuffer *gbuf_stderr;
	MlGrowBuffer *gbuf_report;
	double *values;

	char *cmdline;
	int32_t exec_timeout;
} PARAMETRICstate;


mlPointer
ml_provider_parametric_init (MlConfig *config)
{
	PARAMETRICstate *s = ml_new (PARAMETRICstate);
	s->gbuf_stdout = ml_grow_buffer_new (20);
	s->gbuf_stderr = ml_grow_buffer_new (10);
	s->gbuf_report = ml_grow_buffer_new (1);
	s->values = ml_new_n (double, ML_PROVIDER_PARAMETRIC_IFACE.n_data);

	s->cmdline = NULL;
	s->exec_timeout = 500;

	ml_config_add_entry (config,
		"cmdline",
		ML_VALUE_TYPE_STRING,
		&s->cmdline,
		_("Command line"),
		_("Complete shell-like command line to execute.")
	);

	ml_config_add_entry (config,
		"exec_timeout",
		ML_VALUE_TYPE_INT32,
		&s->exec_timeout,
		_("Execution timeout"),
		_("Number of milliseconds to wait before terminating child process. Set to -1 to wait indefinitely.")
	);

	return s;
}

#define MULTIPLIER 100

void
ml_provider_parametric_get (MlGraphContext *context)
{
	PARAMETRICstate *s = (PARAMETRICstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	ml_graph_context_assert_with_message (context, s->cmdline != NULL && s->cmdline[0] != '\0', _("Command line is empty"));

	int exit_status;
	bool success = ml_execute_cmdline_sync (s->cmdline, NULL, s->exec_timeout, s->gbuf_stdout, s->gbuf_stderr, s->gbuf_report, &exit_status);
	ml_graph_context_assert_with_message (context, success, "%s", ml_grow_buffer_get_string (s->gbuf_report));

	ml_graph_context_assert_with_message (context, exit_status == 0, _("Command has exited with nonzero status code (%d)"), exit_status);

	memset (s->values, 0, ml_graph_context_get_n_data (context) * sizeof (double));

	size_t n = sscanf(ml_grow_buffer_get_string (s->gbuf_stdout), "%lf %lf %lf %lf", &s->values[0], &s->values[1], &s->values[2], &s->values[3]);

	ml_graph_context_assert_with_message (context, n > 0, _("Command did not return valid numbers"));

	for (unsigned i = 0; i<MIN(4,n); i++)
		ml_graph_context_set_data (context, i, MULTIPLIER * s->values[i]);
}

void
ml_provider_parametric_destroy (mlPointer provider_data)
{
	PARAMETRICstate *s = (PARAMETRICstate*)provider_data;

	if_likely (s != NULL) {
		free (s->cmdline);
		ml_grow_buffer_destroy (s->gbuf_stdout, true);
		ml_grow_buffer_destroy (s->gbuf_stderr, true);
		ml_grow_buffer_destroy (s->gbuf_report, true);
		free (s->values);
		free (s);
	}
}

size_t
ml_provider_parametric_sizeof (mlPointer provider_data)
{
	PARAMETRICstate *s = (PARAMETRICstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	size_t size = sizeof (PARAMETRICstate);
	size += ml_grow_buffer_sizeof (s->gbuf_stdout);
	size += ml_grow_buffer_sizeof (s->gbuf_stderr);
	size += ml_grow_buffer_sizeof (s->gbuf_report);
	size += ml_string_sizeof (s->cmdline);

	return size;
}

void
ml_provider_parametric_caption (MlCaption *caption, ML_UNUSED MlDataset *ds, mlPointer provider_data)
{
	PARAMETRICstate *s = (PARAMETRICstate*)provider_data;
	if_unlikely (s == NULL)
		return;

	ml_caption_set (caption, ML_CAPTION_COMPONENT_HEADER, _("Command line: %s"), s->cmdline);
	ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("Values: [%0.02f, %0.02f, %0.02f, %0.02f]"),
		s->values[0], s->values[1], s->values[2], s->values[3]);

	ml_caption_set (caption, ML_CAPTION_COMPONENT_FOOTER, "%s", ml_grow_buffer_get_string (s->gbuf_stderr));
}
