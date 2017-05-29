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

#include <config.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <libintl.h>
#include <multiload.h>


#define APP_NAME "multiload-debug-collector"
#define _(str) (gettext(str))

void
print_usage ()
{
	// TRANSLATORS: program usage line
	printf (_("USAGE: %s [output.zip]"), APP_NAME);
	printf ("\n");
}

void
print_message (const char *fmt, ...)
{
	printf ("%s: ", APP_NAME);

	va_list va;
	va_start (va, fmt);
	vprintf (fmt, va);
	va_end (va);

	printf ("\n");
}

int
main (int argc, const char *argv[])
{
	if (argc != 2) {
		print_usage();
		exit(1);
	}

	// setlocale
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);

	const char *zip_filename = argv[1];

	// TRANSLATORS: %s is a filename
	print_message (_("Collecting data into %s"), zip_filename);

	if (multiload_debug_to_zip (NULL, zip_filename, 9)) {
		print_message (_("Success"));
		return 0;
	} else  {
		print_message (_("Failure"));
		return 1;
	}
}
