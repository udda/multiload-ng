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

#include <multiload-server.h>


// Default port: ASCII 'ML' [0x4D 0x4C]
#define MLSRV_DEFAULT_PORT 19788

// Default values to use with new Multiload
#define MLSRV_DEFAULT_SIZE 200
#define MLSRV_DEFAULT_ORIENTATION "vertical"
#define MLSRV_DEFAULT_PADDING 0


#define APPNAME "multiload-server"

int
main (int argc, char *argv[])
{
	// parameter definitions
	enum {
		MLSRV_PARAMETER_HELP			= 1 << 0,
		MLSRV_PARAMETER_VERBOSE			= 1 << 1,
		MLSRV_PARAMETER_FILE			= 1 << 2,
		MLSRV_PARAMETER_PORT			= 1 << 3,
		MLSRV_PARAMETER_SIZE			= 1 << 4,
		MLSRV_PARAMETER_ORIENTATION		= 1 << 5,
		MLSRV_PARAMETER_PADDING			= 1 << 6,
		MLSRV_PARAMETER_BASIC_AUTH		= 1 << 7,
		MLSRV_PARAMETER_DIGEST_AUTH		= 1 << 8
	};

	// parameter containers
	bool verbose = false;
	uint16_t port			= MLSRV_DEFAULT_PORT;
	int size				= MLSRV_DEFAULT_SIZE;
	char orientation[20]	= MLSRV_DEFAULT_ORIENTATION;
	int padding				= MLSRV_DEFAULT_PADDING;
	char file[PATH_MAX]		= {'\0'};
	char *basic_auth[2]		= { NULL, NULL };
	char *digest_auth[2]	= { NULL, NULL };

	// https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Options.html
	struct option long_options[] = {
		{"file",			required_argument,	NULL, MLSRV_PARAMETER_FILE},
		{"port",			required_argument,	NULL, MLSRV_PARAMETER_PORT},
		{"size",			required_argument,	NULL, MLSRV_PARAMETER_SIZE},
		{"orientation",		required_argument,	NULL, MLSRV_PARAMETER_ORIENTATION},
		{"padding",			required_argument,	NULL, MLSRV_PARAMETER_PADDING},
		{"basic-auth",		required_argument,	NULL, MLSRV_PARAMETER_BASIC_AUTH},
		{"digest-auth",		required_argument,	NULL, MLSRV_PARAMETER_DIGEST_AUTH},
		{"verbose",			no_argument,		NULL, MLSRV_PARAMETER_VERBOSE},
		{"help",			no_argument,		NULL, MLSRV_PARAMETER_HELP},
		{0, 0, 0, 0}
	};
	int parameters_mask = 0;

	// parse parameters
	int c;
	while ((c = getopt_long (argc, argv, "", long_options, NULL)) != -1) {
		parameters_mask |= c;

		switch (c) {
			case MLSRV_PARAMETER_FILE: {
				strncpy (file, optarg, sizeof(file));
				break;
			}

			case MLSRV_PARAMETER_PORT: {
				int p = atoi (optarg);
				if (p >= IPPORT_USERRESERVED && p <= UINT16_MAX) {
					port = (uint16_t)p;
				} else {
					multiload_server_log_failure (MULTILOAD_SERVER_LOG_INITIALIZATION, _("Invalid port number (%s). Port number must be between %d and %d."), optarg, IPPORT_USERRESERVED, UINT16_MAX);
					exit (1);
				}
				break;
			}

			case MLSRV_PARAMETER_SIZE: {
				int s = atoi (optarg);
				if (s > 0) {
					size = s;
				} else {
					multiload_server_log_failure (MULTILOAD_SERVER_LOG_INITIALIZATION, _("Invalid Multiload size (%s)"), optarg);
					exit (1);
				}
				break;
			}

			case MLSRV_PARAMETER_ORIENTATION: {
				strncpy (orientation, optarg, sizeof(orientation));
				break;
			}

			case MLSRV_PARAMETER_PADDING: {
				int d = atoi (optarg);
				if (d > 0) {
					padding = d;
				} else {
					multiload_server_log_failure (MULTILOAD_SERVER_LOG_INITIALIZATION, _("Invalid Multiload padding (%s)"), optarg);
					exit (1);
				}
				break;
			}

			case MLSRV_PARAMETER_BASIC_AUTH: {
				if (!parse_credentials (optarg, &basic_auth[0], &basic_auth[1])) {
					multiload_server_log_failure (MULTILOAD_SERVER_LOG_INITIALIZATION, _("Invalid authentication credentials (%s). Separate username and password with a colon"), optarg);
					exit (1);
				}
				break;
			}

			case MLSRV_PARAMETER_DIGEST_AUTH: {
				if (!parse_credentials (optarg, &digest_auth[0], &digest_auth[1])) {
					multiload_server_log_failure (MULTILOAD_SERVER_LOG_INITIALIZATION, _("Invalid authentication credentials (%s). Separate username and password with a colon"), optarg);
					exit (1);
				}
				break;
			}

			case MLSRV_PARAMETER_VERBOSE:
				verbose = true;
				break;

			case '?': // this is automatically triggered on unrecognized parameters
			case MLSRV_PARAMETER_HELP:
				// TRANSLATORS: help screen: %s is application name
				fprintf(stderr, _("USAGE: %s"), APPNAME);
				// TRANSLATORS: argument for --file parameter
				fprintf(stderr, "\n  [--file <%s>]", _("path"));
				// TRANSLATORS: argument for --port parameter
				fprintf(stderr, "\n  [--port <%s>]", _("number"));
				// TRANSLATORS: argument for --size and --padding parameters
				fprintf(stderr, "\n  [--size <%s>]", _("pixels"));
				fprintf(stderr, "\n  [--padding <%s>]", _("pixels"));
				// TRANSLATORS: argument for --orientation parameter
				fprintf(stderr, "\n  [--orientation <%s>]", _("orientation"));
				// TRANSLATORS: argument for --basic-auth and --digest-auth parameters - keep the colon it's required
				fprintf(stderr, "\n  [--basic-auth <%s> | --digest-auth <%s>]", _("username:password"), _("username:password"));
				fprintf(stderr, "\n  [--verbose]");
				fprintf(stderr, "\n");
				exit (c=='?' ? 1 : 0);

			default:
				break;
		}
	}


	Multiload *ml;

	if (file[0] == '\0' || access (file, F_OK) != 0) {
		if (file[0] == '\0')
			multiload_server_log_begin (MULTILOAD_SERVER_LOG_INITIALIZATION, _("Initializing empty Multiload"));
		else
			multiload_server_log_begin (MULTILOAD_SERVER_LOG_INITIALIZATION, _("'%s' does not exists, initializing empty Multiload"));

		ml = multiload_new (size, orientation, padding);
	} else {
		multiload_server_log_begin (MULTILOAD_SERVER_LOG_INITIALIZATION, _("Initializing Multiload from '%s'"), file);
		ml = multiload_new_from_json_file (file);

		// apply supplied parameters
		if (parameters_mask & MLSRV_PARAMETER_SIZE)
			multiload_set_size (ml, size);

		if (parameters_mask & MLSRV_PARAMETER_ORIENTATION)
			multiload_set_orientation (ml, orientation);

		if (parameters_mask & MLSRV_PARAMETER_PADDING)
			multiload_set_padding (ml, padding);
	}



	if (ml == NULL) {
		multiload_server_log_failure (MULTILOAD_SERVER_LOG_INITIALIZATION, _("Could not initialize Multiload"));
		exit (1);
	}

	MultiloadServer *mlsrv = multiload_server_new (ml, port, file);

	// verbose
	multiload_server_log_set_verbose (verbose);

	// authentication
	if (digest_auth[0] != NULL && digest_auth[1] != NULL)
		multiload_server_set_digest_authentication (mlsrv, digest_auth[0], digest_auth[1]);
	else if (basic_auth[0] != NULL && basic_auth[1] != NULL)
		multiload_server_set_basic_authentication (mlsrv, basic_auth[0], basic_auth[1]);


	// main loop
	while (true) {
		multiload_wait_for_data (ml);
	}

	multiload_server_destroy(mlsrv);
	multiload_destroy (ml);
	return 0;
}
