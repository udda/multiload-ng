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


#ifndef MLSRV_HEADER__HTTP_SERVER_H__INCLUDED
#define MLSRV_HEADER__HTTP_SERVER_H__INCLUDED


typedef struct _MultiloadServer MultiloadServer;


MultiloadServer *
multiload_server_new (Multiload *multiload, uint16_t port, const char *filename);

void
multiload_server_destroy (MultiloadServer *mlsrv);

bool
multiload_server_supports_basic_authentication ();

bool
multiload_server_supports_digest_authentication ();

bool
multiload_server_set_basic_authentication (MultiloadServer *mlsrv, char *username, char *password);

bool
multiload_server_set_digest_authentication (MultiloadServer *mlsrv, char *username, char *password);

int
multiload_server_response (MultiloadServer *mlsrv, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr);

Multiload *
multiload_server_get_multiload (MultiloadServer *mlsrv);

const char *
multiload_server_get_filename (MultiloadServer *mlsrv);

bool
multiload_server_store (MultiloadServer *mlsrv);

bool
multiload_server_reload (MultiloadServer *mlsrv);

bool
multiload_server_is_dirty_data (MultiloadServer *mlsrv);

void
multiload_server_set_dirty_data (MultiloadServer *mlsrv);

void
multiload_server_clear_dirty_data (MultiloadServer *mlsrv);

bool
multiload_server_is_just_started (MultiloadServer *mlsrv);

void
multiload_server_clear_just_started (MultiloadServer *mlsrv);


#endif /* MLSRV_HEADER__HTTP_SERVER_H__INCLUDED */
