/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of multiload-ng.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef __ABOUT_DATA_H__
#define __ABOUT_DATA_H__


#define about_data_progname _("Multiload-ng")
#define about_data_progname_N N_("Multiload-ng")

#define about_data_description \
	_("A system load monitor that graphs processor, memory, and " \
	"swap space use, plus temperature, network and disk activity.")
#define about_data_description_N \
	N_("A system load monitor that graphs processor, memory, and " \
	"swap space use, plus temperature, network and disk activity.")

#define about_data_website "https://udda.github.io/multiload-ng"

#define about_data_preferences_website "https://github.com/udda/multiload-ng/wiki/Configuration"

#define about_data_copyright \
	_("Copyright \xC2\xA9 2016 Mario Cianciolo, " \
	"1999-2012 nandhp, FSF, and others")

#define about_data_license \
	"This program is free software; you can redistribute it and/or modify\n" \
	"it under the terms of the GNU General Public License as published by\n" \
	"the Free Software Foundation; either version 2 of the License, or\n" \
	"(at your option) any later version.\n" \
	"\n" \
	"This program is distributed in the hope that it will be useful,\n" \
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" \
	"GNU General Public License for more details.\n" \
	"\n" \
	"You should have received a copy of the GNU General Public License along\n" \
	"with this program; if not, write to the Free Software Foundation, Inc.,\n" \
	"51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n"

extern const gchar * const about_data_authors[];
extern const gchar * about_data_icon;

#endif /* __ABOUT_DATA_H__ */
