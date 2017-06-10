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


// <editor-fold> Response manipulation
void
multiload_server_response_set_content_length (struct MHD_Response *response, size_t len)
{
	if (response == NULL)
		return;

	char filesize[20];
	snprintf (filesize, sizeof(filesize), "%zu", len);

	MHD_add_response_header (response, "Content-Length", filesize);
}

void
multiload_server_response_set_content_type (struct MHD_Response *response, const char *mimetype)
{
	if (response == NULL || mimetype == NULL)
		return;

	MHD_add_response_header (response, "Content-Type", mimetype);
}

void
multiload_server_response_set_downloadable (struct MHD_Response *response, const char *filename)
{
	if (response == NULL || filename == NULL)
		return;

	char buf[200];
	snprintf(buf, sizeof(buf), "attachment; filename=\"%s\"", filename);

	MHD_add_response_header (response, "Content-Disposition", buf);
}

void
multiload_server_response_set_no_cache (struct MHD_Response *response)
{
	if (response == NULL)
		return;

	MHD_add_response_header (response, "Cache-Control", "private, max-age=0, no-cache");
}
// </editor-fold>


struct MHD_Response *
multiload_server_serve_multiload_image (MultiloadServer *mlsrv)
{
	if (mlsrv == NULL)
		return NULL;

	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	size_t len;
	uint8_t *png = multiload_write_to_png_buffer (ml, &len, 0);

	struct MHD_Response *response = MHD_create_response_from_buffer (len, (void*)png, MHD_RESPMEM_MUST_FREE);

	multiload_server_response_set_content_length	(response, len);
	multiload_server_response_set_content_type		(response, "image/png");
	multiload_server_response_set_no_cache			(response);

	multiload_server_log_success (MULTILOAD_SERVER_LOG_RESPONSE, _("Multiload PNG data [%zu byte]"), len);
	return response;
}

struct MHD_Response *
multiload_server_serve_multiload_command (MultiloadServer *mlsrv, const char *command, struct MHD_Connection *connection)
{
	static struct { char* name; MultiloadServerCommandFunc func; }
	callbackTable[] = {
		{ "status",				multiload_server_command_status },
		{ "library-version",	multiload_server_command_library_version },
		{ "data",				multiload_server_command_data },
		{ "has-file",			multiload_server_command_has_file },
		{ "graph-types",		multiload_server_command_graph_types },
		{ "index-at-coords",	multiload_server_command_index_at_coords },
		{ "create",				multiload_server_command_create },
		{ "delete",				multiload_server_command_delete },
		{ "resume",				multiload_server_command_resume },
		{ "pause",				multiload_server_command_pause },
		{ "move",				multiload_server_command_move },
		{ "caption",			multiload_server_command_caption },
		{ "config-entries",		multiload_server_command_config_entries },
		{ "get-config",			multiload_server_command_get_config },
		{ "set-config",			multiload_server_command_set_config },
		{ "localization",		multiload_server_command_localization },
		{ "save",				multiload_server_command_save },
		{ "reload",				multiload_server_command_reload },
		{ "set-element-size",	multiload_server_command_set_element_size },
		{ "set-graph-border",	multiload_server_command_set_graph_border },
		{ "set-graph-interval",	multiload_server_command_set_graph_interval },
		{ "set-graph-ceiling",	multiload_server_command_set_graph_ceiling },
		{ NULL, NULL }
	};

	if (mlsrv == NULL)
		return NULL;

	char *ret = NULL;

	int i;
	for (i = 0; callbackTable[i].name != NULL; i++) {
		if (!strcmp (callbackTable[i].name, command) && callbackTable[i].func != NULL) {
			ret = callbackTable[i].func (mlsrv, connection);
			if (ret == NULL)
				ret = strdup("");
			break;
		}
	}


	if (ret == NULL) {
		multiload_server_log_failure (MULTILOAD_SERVER_LOG_RESPONSE, _("Unknown command: %s"), command);
		return NULL;
	}

	size_t len = strlen (ret);

	struct MHD_Response *response = MHD_create_response_from_buffer (len, ret, MHD_RESPMEM_MUST_FREE);

	multiload_server_response_set_content_length	(response, len);
	multiload_server_response_set_content_type		(response, "text/json");
	multiload_server_response_set_no_cache			(response);

	multiload_server_log_success (MULTILOAD_SERVER_LOG_RESPONSE, _("Command: %s [%zu byte]"), command, len);
	return response;
}

struct MHD_Response *
multiload_server_serve_static_file (const char *filename)
{
	if (filename == NULL)
		return NULL;

	// open file
	int fd = open (filename, O_RDONLY);
	if (fd == 0)
		return NULL;

	// get file size
	size_t len;
	struct stat st;
	if (stat (filename, &st) == 0)
		len = st.st_size;
	else
		return NULL;

	// generate response
	struct MHD_Response *response = MHD_create_response_from_fd (len, fd);
	multiload_server_response_set_content_length (response, len);

	// guess content type based on file extension
	char *pch = strrchr (filename, '.');
	if (pch != NULL) {
		if (!strcmp (pch+1, "css"))
			multiload_server_response_set_content_type (response, "text/css; charset=UTF-8");
		else if (!strcmp (pch+1, "js"))
			multiload_server_response_set_content_type (response, "text/javascript; charset=UTF-8");
		else if (!strcmp (pch+1, "html"))
			multiload_server_response_set_content_type (response, "text/html; charset=UTF-8");
		else if (!strcmp (pch+1, "png"))
			multiload_server_response_set_content_type (response, "image/png");
	}

	multiload_server_log_success (MULTILOAD_SERVER_LOG_RESPONSE, _("Static resource: %s [%zu byte]"), filename, len);
	return response;
}

struct MHD_Response *
multiload_server_serve_unauthorized_page ()
{
	static char page[300];
	static int len = 0;

	if (len == 0)
		len = snprintf (page, sizeof (page), "<!doctype html><html><head><meta charset=\"utf-8\"><title>Multiload-ng</title></head><body>%s</body></html>", _("You are not authorized."));

	struct MHD_Response *response = MHD_create_response_from_buffer (len, page, MHD_RESPMEM_PERSISTENT);

	multiload_server_response_set_content_length	(response, len);
	multiload_server_response_set_content_type		(response, "text/html; charset=UTF-8");

	// don't log this (when using Digest auth, an unauthorized page is returned before every valid request)
	//multiload_server_log_success (MULTILOAD_SERVER_LOG_RESPONSE, _("Unauthorized access page [%zu byte]"), len);
	return response;
}

struct MHD_Response *
multiload_server_serve_multiload_json (MultiloadServer *mlsrv)
{
	if (mlsrv == NULL)
		return NULL;

	Multiload *ml = multiload_server_get_multiload (mlsrv);
	if (ml == NULL)
		return NULL;

	char *json_data = multiload_to_json (ml);
	if (json_data == NULL)
		return NULL;

	size_t len = strlen (json_data);

	struct MHD_Response *response = MHD_create_response_from_buffer (len, json_data, MHD_RESPMEM_MUST_FREE);

	multiload_server_response_set_content_length	(response, len);
	multiload_server_response_set_downloadable		(response, "multiload.json");
	multiload_server_response_set_no_cache			(response);

	multiload_server_log_success (MULTILOAD_SERVER_LOG_RESPONSE, _("Multiload JSON data [%zu byte]"), len);
	return response;
}
