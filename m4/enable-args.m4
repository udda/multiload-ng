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


# MULTILOAD_ARG_DEBUG()
# Parse command line argument --enable-debug
AC_DEFUN([MULTILOAD_ARG_DEBUG],
[
	AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [debug-friendly build]), [], [enable_debug="no"])
	AM_CONDITIONAL(MULTILOAD_ENABLE_DEBUG, test x"$enable_debug" = x"yes")
	m4_define([MULTILOAD_ENABLE_DEBUG], "$enable_debug")
])

# MULTILOAD_ARG_LTO()
# Parse command line argument --enable-lto
AC_DEFUN([MULTILOAD_ARG_LTO],
[
	AC_ARG_ENABLE([lto], AS_HELP_STRING([--enable-lto], [enable Link Time Optimization (LTO), which (when supported) produces smaller and faster binaries]), [], [enable_lto="no"])
	AM_CONDITIONAL(MULTILOAD_ENABLE_LTO, test x"$enable_lto" = x"yes")
 	m4_define([MULTILOAD_ENABLE_LTO], "$enable_lto")
])
