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


#ifndef MLSRV_HEADER__HTTP_RESPONSE_H__INCLUDED
#define MLSRV_HEADER__HTTP_RESPONSE_H__INCLUDED


void
multiload_server_response_set_content_length (struct MHD_Response *response, size_t len);

void
multiload_server_response_set_content_type (struct MHD_Response *response, const char *mimetype);

void
multiload_server_response_set_no_cache (struct MHD_Response *response);


struct MHD_Response *
multiload_server_serve_multiload_image (MultiloadServer *mlsrv);

struct MHD_Response *
multiload_server_serve_multiload_command (MultiloadServer *mlsrv, const char *command, struct MHD_Connection *connection);

struct MHD_Response *
multiload_server_serve_static_file (const char *filename);

struct MHD_Response *
multiload_server_serve_unauthorized_page ();

struct MHD_Response *
multiload_server_serve_multiload_json (MultiloadServer *mlsrv);


#endif /* MLSRV_HEADER__HTTP_RESPONSE_H__INCLUDED */
