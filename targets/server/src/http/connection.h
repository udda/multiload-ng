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


#ifndef MLSRV_HEADER__HTTP_CONNECTION_H__INCLUDED
#define MLSRV_HEADER__HTTP_CONNECTION_H__INCLUDED


const char *
multiload_server_connection_get_parameter_string (struct MHD_Connection *connection, const char *key);

int
multiload_server_connection_get_parameter_int (struct MHD_Connection *connection, const char *key);


#endif /* MLSRV_HEADER__HTTP_CONNECTION_H__INCLUDED */
