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


bool
ml_file_is_readable (const char *path)
{
	if_unlikely (path == NULL)
		return false;

	return access (path, R_OK) == 0;
}

size_t
ml_file_size (const char *path)
{
	if_unlikely (path == NULL)
		return 0;

	if (!ml_file_is_readable (path))
		return 0;

	struct stat st;
	if (stat (path, &st) != 0)
		return 0;

	return st.st_size;
}

void
ml_cleanup_FILE (FILE **pf)
{
	if (pf != NULL) {
		fclose (*pf);
		*pf = NULL;
	}
}
