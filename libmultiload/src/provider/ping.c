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


const MlGraphTypeInterface ML_PROVIDER_PING_IFACE = {
	.name			= "ping",
	.label			= N_("Ping"),
	.description	= N_("Shows ping time for a given host."),

	.hue			= 210,

	.n_data			= 1,
	.dataset_mode	= ML_DATASET_MODE_ABSOLUTE,

	.init_fn		= ml_provider_ping_init,
	.config_fn		= ml_provider_ping_config,
	.get_fn			= ml_provider_ping_get,
	.destroy_fn		= ml_provider_ping_destroy,
	.sizeof_fn		= ml_provider_ping_sizeof,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_ping_caption,

	.helptext		= N_(
		"Requires <i>ping</i> installed in this machine. <br />"
		"For IPv6 to work, both host system and <i>ping</i> executable must support IPv6. IPv4 is always supported."
	)
};


typedef struct {
	char *cmdline;
	MlGrowBuffer *gbuf_stdout;
	MlGrowBuffer *gbuf_report;

	char *host;
} PINGstate;

mlPointer
ml_provider_ping_init (MlConfig *config)
{
	PINGstate *s = ml_new (PINGstate);
	s->cmdline = NULL;
	s->gbuf_stdout = ml_grow_buffer_new (500);
	s->gbuf_report = ml_grow_buffer_new (1);
	s->host = NULL;

	ml_config_add_entry (config,
		"host",
		ML_VALUE_TYPE_STRING,
		&s->host,
		_("Host address"),
		_("Address of remote host to ping. Can be a domain name (eg. <i>google.com</i> or an IP address (eg. <i>216.58.205.78</i>).")
	);

	return s;
}

void
ml_provider_ping_config (MlGraphContext *context)
{
	PINGstate *s = (PINGstate*)ml_graph_context_get_provider_data (context);
	if_unlikely (s == NULL)
		return;

	free (s->cmdline);

	if (s->host != NULL && s->host[0] != '\0')
		s->cmdline = ml_strdup_printf ("env LC_ALL=C ping -c 1 -n -q \"%s\"", s->host);
	else
		s->cmdline = NULL;

	ml_graph_context_set_need_data_reset (context);
}

void
ml_provider_ping_get (MlGraphContext *context)
{
	PINGstate *s = (PINGstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	ml_graph_context_assert_with_message (context, s->cmdline != NULL && s->cmdline[0] != '\0', _("Command line is empty"));

	int exit_status;
	bool success = ml_execute_cmdline_sync (s->cmdline, NULL, -1, s->gbuf_stdout, NULL, s->gbuf_report, &exit_status);

	ml_graph_context_assert_with_message (context, success, _("Unable to run 'ping': %s"), ml_grow_buffer_get_string(s->gbuf_report));

	/* ping exit status:
	 *  0: success
	 *  1: no reply
	 *  2: error (including invalid host) */
	ml_graph_context_assert_with_message (context, exit_status != 1, _("No reply from host %s"), s->host);
	ml_graph_context_assert_with_message (context, exit_status == 0, _("Ping error (exit status %d)"), exit_status);

	char *ping_output = ml_grow_buffer_get_string (s->gbuf_stdout);
	ml_graph_context_assert_with_message (context, !ml_string_is_null_or_empty(ping_output), _("No output from ping"));

	char *pch = strstr (ping_output, "rtt min/avg/max/mdev");
	ml_graph_context_assert_with_message (context, pch != NULL, _("Unexpected output from 'ping' command"));

	unsigned ping_ms, ping_us;
	size_t n = sscanf(pch, "rtt min/avg/max/mdev = %u.%u/%*u.%*u/%*u.%*u/%*u.%*u ms", &ping_ms, &ping_us);
	ml_graph_context_assert_with_message (context, n == 2, _("Unexpected output from 'ping' command"));

	/* To achieve both granularity and large enough max value for 32 bit integers,
	 * we count ping times as 10th of a millisecond. This way max representable time
	 * is 4 days, 23 hours, 18 minutes, 16 seconds, 729.5 milliseconds */
	ml_graph_context_set_data (context, 0, (ping_ms * 10) + (ping_us / 100));
	ml_graph_context_set_max  (context, 50); // min ceiling = 50 ms
}

void
ml_provider_ping_destroy (mlPointer provider_data)
{
	PINGstate *s = (PINGstate*)provider_data;

	if_likely (s != NULL) {
		free (s->cmdline);
		free (s->host);
		ml_grow_buffer_destroy (s->gbuf_stdout, true);
		ml_grow_buffer_destroy (s->gbuf_report, true);
		free (s);
	}
}

size_t
ml_provider_ping_sizeof (mlPointer provider_data)
{
	PINGstate *s = (PINGstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	size_t size = sizeof (PINGstate);

	size += ml_string_sizeof (s->cmdline);
	size += ml_string_sizeof (s->host);
	size += ml_grow_buffer_sizeof (s->gbuf_stdout);
	size += ml_grow_buffer_sizeof (s->gbuf_report);

	return size;
}

void
ml_provider_ping_caption (MlCaption *caption, MlDataset *ds, mlPointer provider_data)
{
	PINGstate *s = (PINGstate*)provider_data;
	if_unlikely (s == NULL)
		return;

	uint32_t val = ml_dataset_get_value (ds, -1, 0);

	// TRANSLATORS: internet address of selected host
	ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("Host name: %s"), s->host);
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");

	// TRANSLATORS: ping time for selected host. "ms" stands for "milliseconds"
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Ping time: %0.01f ms"), (float)val/10.0);
}
