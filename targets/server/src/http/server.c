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


typedef enum {
	MULTILOAD_SERVER_AUTH_NONE,
	MULTILOAD_SERVER_AUTH_BASIC,
	MULTILOAD_SERVER_AUTH_DIGEST
} MultiloadServerAuthenticationType;

static const char *
_authentication_type_to_string (MultiloadServerAuthenticationType type)
{
	if (type == MULTILOAD_SERVER_AUTH_NONE)
		return _("No authentication");

	if (type == MULTILOAD_SERVER_AUTH_BASIC)
		return "Basic";

	if (type == MULTILOAD_SERVER_AUTH_DIGEST)
		return "Digest";

	return _("Unknown authentication type");
}

struct _MultiloadServer {
	Multiload *multiload;

	bool dirty_data;
	bool just_started;
	char *filename;

	uint16_t port;
	struct MHD_Daemon *daemon;

	MultiloadServerAuthenticationType auth_type;
	char *username;
	char *password;
};


#define MULTILOAD_SERVER_THREAD_POOL_SIZE 10
#define MULTILOAD_SERVER_AUTH_REALM "Multiload-ng Realm"
#define MULTILOAD_SERVER_AUTH_OPAQUE "ac64fd5efd6b4c548e4dafdef4b219d7b897a"


// <editor-fold> Constructor / Destructor
MultiloadServer *
multiload_server_new (Multiload *multiload, uint16_t port, const char *filename)
{
	MultiloadServer *mlsrv = (MultiloadServer*)calloc (1, sizeof(MultiloadServer));
	if (mlsrv == NULL) {
		printf ("Out of memory... Aborting.\n");
		abort ();
	}

	mlsrv->multiload = multiload;
	mlsrv->dirty_data = true;
	mlsrv->just_started = true;
	mlsrv->port = port;
	mlsrv->daemon = MHD_start_daemon(
		MHD_USE_SELECT_INTERNALLY | MHD_USE_DUAL_STACK,
		mlsrv->port,
		NULL, NULL, // MHD_AcceptPolicyCallback + user_data
		(MHD_AccessHandlerCallback)multiload_server_response, mlsrv, // MHD_AccessHandlerCallback + user_data
		// begin vararg options (terminate with MHD_OPTION_END)
		MHD_OPTION_CONNECTION_MEMORY_LIMIT,			(size_t)(1024 * 256),
		MHD_OPTION_CONNECTION_MEMORY_INCREMENT,		(size_t)(1024 * 2),
		MHD_OPTION_CONNECTION_TIMEOUT,				0, // in seconds (0: disabled)
		MHD_OPTION_THREAD_POOL_SIZE,				MULTILOAD_SERVER_THREAD_POOL_SIZE,
		MHD_OPTION_END
	);

	if (filename != NULL)
		mlsrv->filename = strdup(filename);

	if (mlsrv->daemon == NULL) {
		multiload_server_destroy (mlsrv);
		return NULL;
	}

	mlsrv->auth_type = MULTILOAD_SERVER_AUTH_NONE;

	multiload_server_log_success (MULTILOAD_SERVER_LOG_INITIALIZATION, _("Server started, listening at port %d"), port);

	return mlsrv;
}

void
multiload_server_destroy (MultiloadServer *mlsrv)
{
	if (mlsrv == NULL)
		return;

	if (mlsrv->daemon != NULL)
		MHD_stop_daemon(mlsrv->daemon);

	free (mlsrv->filename);
	free (mlsrv->username);
	free (mlsrv->password);

	free (mlsrv);

	multiload_server_log_success (MULTILOAD_SERVER_LOG_FINALIZATION, _("Server destroyed"));
}
// </editor-fold>

// <editor-fold> Authentication
bool
multiload_server_supports_basic_authentication ()
{
	return MHD_is_feature_supported (MHD_FEATURE_BASIC_AUTH) == MHD_YES;
}

bool
multiload_server_supports_digest_authentication ()
{
	return MHD_is_feature_supported (MHD_FEATURE_DIGEST_AUTH) == MHD_YES;
}

bool
multiload_server_set_basic_authentication (MultiloadServer *mlsrv, char *username, char *password)
{
	if (mlsrv == NULL)
		return false;

	if (!multiload_server_supports_basic_authentication ())
		return false;

	free (mlsrv->username);
	free (mlsrv->password);

	mlsrv->auth_type = MULTILOAD_SERVER_AUTH_BASIC;
	mlsrv->username = strdup (username);
	mlsrv->password = strdup (password);

	char *temp = text_to_password (password);
	multiload_server_log_success (MULTILOAD_SERVER_LOG_AUTHENTICATION, _("Set basic authentication (username: %s, password: %s)"), username, temp);
	free (temp);

	return true;
}

bool
multiload_server_set_digest_authentication (MultiloadServer *mlsrv, char *username, char *password)
{
	if (mlsrv == NULL)
		return false;

	if (!multiload_server_supports_digest_authentication ())
		return false;

	free (mlsrv->username);
	free (mlsrv->password);

	mlsrv->auth_type = MULTILOAD_SERVER_AUTH_DIGEST;
	mlsrv->username = strdup (username);
	mlsrv->password = strdup (password);

	char *temp = text_to_password (password);
	multiload_server_log_success (MULTILOAD_SERVER_LOG_AUTHENTICATION, _("Set digest authentication (username: %s, password: %s)"), username, temp);
	free (temp);

	return true;
}
// </editor-fold>

int
multiload_server_response (MultiloadServer *mlsrv, struct MHD_Connection *connection, const char *url, const char *method, ML_UNUSED const char *version, ML_UNUSED const char *upload_data, ML_UNUSED size_t *upload_data_size, void **ptr)
{
	if (mlsrv == NULL)
		return MHD_NO;

	// only GET allowed
	if (strcmp(method, "GET") != 0)
		return MHD_NO;

	// do not answer the first request for each connection
	if (*ptr == NULL) {
		*ptr = connection;
		return MHD_YES;
	}


	// authentication
	if (mlsrv->username != NULL && mlsrv->password != NULL) {

		switch (mlsrv->auth_type) {

			// Basic Authentication
			case MULTILOAD_SERVER_AUTH_BASIC: {
				char *password = NULL;
				char *username = MHD_basic_auth_get_username_password (connection, &password);

				bool valid_credentials;

				if (password == NULL || username == NULL)
					valid_credentials = false;
				else if (strcmp (username, mlsrv->username) != 0 || strcmp (password, mlsrv->password) != 0)
					valid_credentials = false;
				else
					valid_credentials = true;

				free (username);
				free (password);

				if (!valid_credentials) {
					struct MHD_Response *response = multiload_server_serve_unauthorized_page ();

					multiload_server_log_failure (MULTILOAD_SERVER_LOG_AUTHENTICATION, _("Basic Authentication failed: invalid credentials"));

					int ret = MHD_queue_basic_auth_fail_response (connection, MULTILOAD_SERVER_AUTH_REALM, response);
					MHD_destroy_response (response);
					return ret;
				}
				break;
			}

			// Digest Autnehtication
			case MULTILOAD_SERVER_AUTH_DIGEST: {
				int val = MHD_NO;

				char *username = MHD_digest_auth_get_username(connection);

				bool valid_credentials;

				if (username == NULL || strcmp (username, mlsrv->username) != 0) {
					valid_credentials = false;
				} else {
					val = MHD_digest_auth_check(connection, MULTILOAD_SERVER_AUTH_REALM, username, mlsrv->password, 300);
					free(username);

					if (val == MHD_INVALID_NONCE || val == MHD_NO)
						valid_credentials = false;
					else
						valid_credentials = true;
				}

				if (!valid_credentials) {
					struct MHD_Response *response = multiload_server_serve_unauthorized_page ();

					int return_status = (val == MHD_INVALID_NONCE) ? MHD_YES : MHD_NO;
					if (return_status == MHD_NO)
						multiload_server_log_failure (MULTILOAD_SERVER_LOG_AUTHENTICATION, _("Digest Authentication failed: invalid credentials"));

					int ret = MHD_queue_auth_fail_response(connection, MULTILOAD_SERVER_AUTH_REALM, MULTILOAD_SERVER_AUTH_OPAQUE, response, return_status);
					MHD_destroy_response(response);
					return ret;
				}
				break;
			}

			default:
				break;

		}
	}


	// log
	struct sockaddr *addr = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
	char ip[MAX(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)];
	sockaddr_get_address (addr, ip, sizeof(ip));
	const char *protocol = sockaddr_get_protocol (addr);
	int port = sockaddr_get_port (addr);
	const char *authentication = _authentication_type_to_string (mlsrv->auth_type);
	multiload_server_log_success (MULTILOAD_SERVER_LOG_CONNECTION, _("Incoming %s request from %s, port %d (authentication: %s)"), protocol, ip, port, authentication);


	// parse URL and create response
	struct MHD_Response *response;
	if (     !strcmp (url, "/"))
		response = multiload_server_serve_static_file ("static/index.html");
	else if (!strcmp (url, "/multiload.png"))
		response = multiload_server_serve_multiload_image (mlsrv);
	else if (!strcmp (url, "/favicon.png"))
		response = multiload_server_serve_static_file ("/usr/share/icons/hicolor/32x32/apps/multiload-ng.png");
	else if (!strcmp (url, "/logo.png"))
		response = multiload_server_serve_static_file ("/usr/share/icons/hicolor/48x48/apps/multiload-ng.png");
	else if (!strncmp (url, "/command/", 9))
		response = multiload_server_serve_multiload_command (mlsrv, url+9, connection);
	else if (!strncmp (url, "/static/", 8))
		response = multiload_server_serve_static_file (url+1);
	else if (!strncmp (url, "/multiload.json", 8))
		response = multiload_server_serve_multiload_json (mlsrv);
	else {
		multiload_server_log_failure (MULTILOAD_SERVER_LOG_CONNECTION, _("URL not handled: %s"), url);
		return MHD_NO;
	}

	if (response == NULL) {
		multiload_server_log_failure (MULTILOAD_SERVER_LOG_CONNECTION, _("Cannot complete request for '%s'"), url);
		return MHD_NO;
	}

	// return response
	int ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);
	return ret;
}

Multiload *
multiload_server_get_multiload (MultiloadServer *mlsrv)
{
	if (mlsrv == NULL)
		return NULL;

	return mlsrv->multiload;
}

const char *
multiload_server_get_filename (MultiloadServer *mlsrv)
{
	if (mlsrv == NULL)
		return NULL;

	return mlsrv->filename;
}

bool
multiload_server_store (MultiloadServer *mlsrv)
{
	if (mlsrv == NULL || mlsrv->multiload == NULL || mlsrv->filename == NULL)
		return false;

	return multiload_to_json_file (mlsrv->multiload, mlsrv->filename);
}

bool
multiload_server_reload (MultiloadServer *mlsrv)
{
	if (mlsrv == NULL || mlsrv->filename == NULL)
		return false;

	Multiload *ml = multiload_new_from_json_file (mlsrv->filename);
	if (ml == NULL)
		return false;

	if (mlsrv->multiload != NULL)
		multiload_destroy (mlsrv->multiload);

	mlsrv->multiload = ml;
	return true;
}

bool
multiload_server_is_dirty_data (MultiloadServer *mlsrv)
{
	if (mlsrv == NULL)
		return false;

	return mlsrv->dirty_data;
}

void
multiload_server_set_dirty_data (MultiloadServer *mlsrv)
{
	if (mlsrv == NULL)
		return;

	mlsrv->dirty_data = true;
}

void
multiload_server_clear_dirty_data (MultiloadServer *mlsrv)
{
	if (mlsrv == NULL)
		return;

	mlsrv->dirty_data = false;
}

bool
multiload_server_is_just_started (MultiloadServer *mlsrv)
{
	if (mlsrv == NULL)
		return false;

	return mlsrv->just_started;
}

void
multiload_server_clear_just_started (MultiloadServer *mlsrv)
{
	if (mlsrv == NULL)
		return;

	mlsrv->just_started = false;
}
