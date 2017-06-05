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


#define CASE_TYPE_TO_IFACE(t) case ML_GRAPH_TYPE_##t : { return &ML_PROVIDER_##t##_IFACE; }

const MlGraphTypeInterface*
ml_graph_type_interface_get (MlGraphType type) {
	switch (type) {
		CASE_TYPE_TO_IFACE(BATTERY)
		CASE_TYPE_TO_IFACE(CPU)
		CASE_TYPE_TO_IFACE(CPUFREQ)
		CASE_TYPE_TO_IFACE(DISKIO)
		CASE_TYPE_TO_IFACE(ENTROPY)
		CASE_TYPE_TO_IFACE(JAVASCRIPT)
		CASE_TYPE_TO_IFACE(INTRRATE)
		CASE_TYPE_TO_IFACE(LOADAVG)
		CASE_TYPE_TO_IFACE(LOADAVG_FULL)
		CASE_TYPE_TO_IFACE(NET)
		CASE_TYPE_TO_IFACE(PARAMETRIC)
		CASE_TYPE_TO_IFACE(PING)
		CASE_TYPE_TO_IFACE(PROCRATE)
		CASE_TYPE_TO_IFACE(RAM)
		CASE_TYPE_TO_IFACE(RANDOM)
		CASE_TYPE_TO_IFACE(SOCKETS)
		CASE_TYPE_TO_IFACE(STORAGE)
		CASE_TYPE_TO_IFACE(SWAP)
		CASE_TYPE_TO_IFACE(TESTFAIL)
		CASE_TYPE_TO_IFACE(TEMPERATURE)
		CASE_TYPE_TO_IFACE(THREADS)
		CASE_TYPE_TO_IFACE(WIFI)

		default:
			return NULL;
	}
}

const char*
ml_graph_type_to_string (MlGraphType type)
{
	const MlGraphTypeInterface* iface = ml_graph_type_interface_get (type);
	if_unlikely (iface == NULL)
		return NULL;

	return iface->name;
}

MlGraphType
ml_graph_type_parse (const char *def)
{
	if_unlikely (def == NULL)
		return ML_INVALID;

	for (int i = 0; i < ML_GRAPH_TYPE_MAX; i++) {
		if (ml_string_equals (def, ml_graph_type_to_string(i), true))
			return (MlGraphType)i;
	}

	return ML_INVALID;
}

cJSON *
ml_graph_type_to_json (MlGraphType type)
{
	const char *str = ml_graph_type_to_string (type);
	if_unlikely (type == ML_INVALID)
		return NULL;

	return cJSON_CreateString (str);
}

MlGraphType
ml_graph_type_parse_json (cJSON* obj)
{
	if_unlikely (obj == NULL)
		return ML_INVALID;

	return ml_graph_type_parse (obj->valuestring);
}
