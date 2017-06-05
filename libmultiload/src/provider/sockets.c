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


const MlGraphTypeInterface ML_PROVIDER_SOCKETS_IFACE = {
	.name			= "sockets",
	.label			= N_("Sockets in use"),
	.description	= N_("Shows active sockets, split by type (TCP/UDP) and protocol (IPv4/IPv6)."),

	.hue			= 130,

	.n_data			= 2,
	.dataset_mode	= ML_DATASET_MODE_ABSOLUTE,

	.init_fn		= ml_provider_sockets_init,
	.config_fn		= ml_provider_sockets_config,
	.get_fn			= ml_provider_sockets_get,
	.destroy_fn		= free,
	.sizeof_fn		= ml_provider_sockets_sizeof,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_sockets_caption,

	.helptext		= N_(
		"Some distro disable IPv6 support. Selecting IPv6 in these systems will cause graph failure."
	)
};


enum { SOCK_TCP=0, SOCK_UDP=1 };

enum {
	IP_4		= 1,
	IP_6		= 2,
	IP_BOTH		= IP_4|IP_6
};

typedef struct {
	int32_t ip_version;

	uint32_t sockets_4[2];
	uint32_t sockets_6[2];
} SOCKETSstate;


#define PATH_SOCKSTAT4 "/proc/net/sockstat"
#define PATH_SOCKSTAT6 "/proc/net/sockstat6"

mlPointer
ml_provider_sockets_init (MlConfig *config)
{
	SOCKETSstate *s = ml_new (SOCKETSstate);
	s->ip_version = IP_BOTH;

	ml_config_add_entry_with_bounds (config,
		"ip_version",
		ML_VALUE_TYPE_INT32,
		&s->ip_version,
		_("IP version"),
		_("Enter <b>1</b> to count only IPv4 sockets, <b>2</b> to count only IPv6 sockets, or <b>3</b> to count both."),
		1, 3
	);

	return s;
}

void
ml_provider_sockets_config (MlGraphContext *context)
{
	SOCKETSstate *s = (SOCKETSstate*)ml_graph_context_get_provider_data (context);
	if_likely (s != NULL) {
		if_unlikely (s->ip_version != IP_4 && s->ip_version != IP_6 && s->ip_version != IP_BOTH)
			s->ip_version = IP_BOTH;
	}
}

void
ml_provider_sockets_get (MlGraphContext *context)
{
	SOCKETSstate *s = (SOCKETSstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	s->sockets_4[SOCK_TCP] = 0;
	s->sockets_4[SOCK_UDP] = 0;
	s->sockets_6[SOCK_TCP] = 0;
	s->sockets_6[SOCK_UDP] = 0;

	for (int i = 1; i <= 2; i++) { // IP_4 = 1, IP_6 = 2
		FILE *f;

		if (i == IP_4 && (s->ip_version & IP_4)) {
			f = fopen (PATH_SOCKSTAT4, "r");
			ml_graph_context_assert_with_message (context, f != NULL, _("Socket data for IPv4 is missing"));
		} else if (i == IP_6 && (s->ip_version & IP_6)) {
			f = fopen (PATH_SOCKSTAT6, "r");
			ml_graph_context_assert_with_message (context, f != NULL, _("Socket data for IPv6 is missing"));
		} else
			continue;

		char *pch, *endptr;
		char *line = NULL;
		size_t n = 0;

		while (getline(&line, &n, f) >= 0) {
			if (ml_string_has_prefix(line, "TCP") || ml_string_has_prefix(line, "UDP")) {
				if ((pch = strstr (line, "inuse")) == NULL)
					continue;

				uint32_t sockets = (uint32_t)ml_ascii_strtoll (pch+6, &endptr, 0);
				if (pch+6 == endptr) {
					free (line);
					fclose (f);
				}
				ml_graph_context_assert_with_message (context, pch+6 != endptr, _("Wrong line format"));

				int t = line[0] == 'T' ? SOCK_TCP : SOCK_UDP;
				if (i == IP_4)
					s->sockets_4[t] += sockets;
				else // IP_6
					s->sockets_6[t] += sockets;
			}
		}
		free (line);
		fclose (f);
	}

	ml_graph_context_set_data (context, SOCK_TCP, s->sockets_4[SOCK_TCP] + s->sockets_6[SOCK_TCP]);
	ml_graph_context_set_data (context, SOCK_UDP, s->sockets_4[SOCK_UDP] + s->sockets_6[SOCK_UDP]);
	ml_graph_context_set_max  (context, 5); // min ceiling = 5
}

size_t
ml_provider_sockets_sizeof (mlPointer provider_data)
{
	SOCKETSstate *s = (SOCKETSstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	return sizeof (SOCKETSstate);
}


void
ml_provider_sockets_caption (MlCaption *caption, ML_UNUSED MlDataset *ds, mlPointer provider_data)
{
	SOCKETSstate *s = (SOCKETSstate*)provider_data;
	if_unlikely (s == NULL)
		return;

	switch (s->ip_version) {
		case IP_4:
			ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("TCP sockets: %u (IPv4)"), s->sockets_4[SOCK_TCP]);
			ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("UDP sockets: %u (IPv4)"), s->sockets_4[SOCK_UDP]);
			break;
		case IP_6:
			ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("TCP sockets: %u (IPv6)"), s->sockets_6[SOCK_TCP]);
			ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("UDP sockets: %u (IPv6)"), s->sockets_6[SOCK_UDP]);
			break;
		case IP_BOTH:
			ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("TCP sockets: %u (IPv4), %u (IPv6)"), s->sockets_4[SOCK_TCP], s->sockets_6[SOCK_TCP]);
			ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("UDP sockets: %u (IPv4), %u (IPv6)"), s->sockets_4[SOCK_UDP], s->sockets_6[SOCK_UDP]);
			break;
	}
}
