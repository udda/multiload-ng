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


cJSON *
ml_cJSON_GetObjectItem (const cJSON * const object, const char * const string)
{
	/* Use this function instead of cJSON_GetObjectItem to display
	 * a warning message if specified key does not exists */

	if_unlikely (object == NULL || string == NULL)
		return NULL;

	cJSON *obj = cJSON_GetObjectItem (object, string);
	if (obj == NULL)
		ml_warning ("Object \"%s\" doesn't have a child named \"%s\"", object->string, string);

	return obj;
}
