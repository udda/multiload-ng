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


#ifndef MLSRV_HEADER__HTTP_COMMAND_H__INCLUDED
#define MLSRV_HEADER__HTTP_COMMAND_H__INCLUDED


typedef char* (*MultiloadServerCommandFunc)(MultiloadServer *mlsrv, struct MHD_Connection *connection);


char *
multiload_server_command_status (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_has_file (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_library_version (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_data (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_graph_types (MultiloadServer *mlsrv, ML_UNUSED struct MHD_Connection *connection);

char *
multiload_server_command_index_at_coords (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_create (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_delete (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_resume (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_pause (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_move (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_caption (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_config_entries (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_get_config (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_set_config (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_localization (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_save (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_reload (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_set_element_size (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_set_graph_border (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_set_graph_interval (MultiloadServer *mlsrv, struct MHD_Connection *connection);

char *
multiload_server_command_set_graph_ceiling (MultiloadServer *mlsrv, struct MHD_Connection *connection);


#endif /* MLSRV_HEADER__HTTP_COMMAND_H__INCLUDED */
