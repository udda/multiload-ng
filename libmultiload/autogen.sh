#!/bin/sh

# Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
#
# This file is part of Multiload-ng.
#
# Multiload-ng is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Multiload-ng is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.


show_error_and_exit ()
{
	echo "Last command produced an error."
	exit 1
}


echo "Preparing build tree..."

mkdir -p m4

echo "Running autopoint   (gettext support)..."
autopoint --force                        || show_error_and_exit

echo "Running aclocal     (external M4 macro support)..."
aclocal                                  || show_error_and_exit

echo "Running libtoolize  (libtool support)..."
libtoolize --force --copy --quiet        || show_error_and_exit

echo "Running autoheader  (config.h template)..."
autoheader --force                       || show_error_and_exit

echo "Running automake    (Makefile template)..."
automake --force --copy --add-missing    || show_error_and_exit

echo "Running autoconf    (generate ./configure script)..."
autoconf --force                         || show_error_and_exit


echo "Autogen script succeeded. Build tree is ready."
