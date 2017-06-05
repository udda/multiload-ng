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


mlPointer
ml_enforce_calloc (size_t nmemb, size_t size)
{
	mlPointer ptr = calloc (nmemb, size);

	if (ptr == NULL) {
		printf ("Multiload-ng: out of memory... Aborting.\n");
		abort ();
	}

	return ptr;
}

mlPointer
ml_enforce_realloc (mlPointer ptr, size_t size)
{
	mlPointer newptr = realloc (ptr, size);

	if (newptr == NULL) {
		printf ("Multiload-ng: out of memory... Aborting.\n");
		abort ();
	}

	return newptr;
}


void
ml_cleanup_string (char **str)
{
	if (str != NULL) {
		free (*str);
		*str = NULL;
	}
}

void
ml_cleanup_pointer (mlPointer *pptr)
{
	if (pptr != NULL) {
		free (*pptr);
		*pptr = NULL;
	}
}
