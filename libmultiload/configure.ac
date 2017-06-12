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


AC_PREREQ([2.69])
AC_CONFIG_MACRO_DIRS([m4 ../m4])

# Change package name here. It will define MULTILOAD_PACKAGE
MULTILOAD_INIT([libmultiload])
AC_INIT(MULTILOAD_PACKAGE, MULTILOAD_VERSION, MULTILOAD_EMAIL, MULTILOAD_PACKAGE, MULTILOAD_URL)


# Useful data in config.h
AC_DEFINE_UNQUOTED([CONFIGURE_FLAGS],["$*"],[Flags passed to configure])
AC_DEFINE_UNQUOTED([PACKAGE_COPYRIGHT_NOTICE],"$MULTILOAD_COPYRIGHT",[Copyright notice])


# Common initialization
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/multiload.h])
AC_COPYRIGHT([$MULTILOAD_COPYRIGHT])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_SILENT_RULES(yes)


# Checks for programs.
AC_PROG_CC
PKG_PROG_PKG_CONFIG
AC_PATH_PROG([DOXYGEN], [doxygen], [no])


# Checks for libraries.
#AX_PKG_CHECK_MODULES([JSON_C], [], [json-c >= 0.11.0])


# Checks for header files.
AC_CHECK_HEADERS([dirent.h inttypes.h mntent.h stdbool.h stdlib.h string.h pthread.h time.h unistd.h arpa/inet.h net/if.h sys/statvfs.h sys/time.h sys/utsname.h],
	[], [AC_MSG_ERROR([One or more required headers are missing.])])


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T


# Checks for library functions.


# Build time options
AC_ARG_ENABLE([lto], AS_HELP_STRING([--enable-lto], [enable Link Time Optimization (LTO), which (when supported) produces smaller and faster binaries]), [], [enable_lto="no"])
AM_CONDITIONAL(LTO, test x"$enable_lto" = x"yes")

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [debug-friendly build]), [], [enable_debug="no"])
AM_CONDITIONAL(DEBUG, test x"$enable_debug" = x"yes")


# Libtool
AM_PROG_AR
LT_INIT([disable-static])


# Gettext
AM_GNU_GETTEXT_REQUIRE_VERSION([0.18.1])
AM_GNU_GETTEXT([external])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], "$PACKAGE_NAME", [Gettext package name.])


# Dependencies
PKG_INSTALLDIR


# Output
AC_CONFIG_FILES([
	doc/Makefile
	icon/Makefile
	po/Makefile.in
	src/resources/Makefile
	Makefile
	libmultiload-2.pc
])


AC_OUTPUT

echo
echo '  __  __       _ _   _ _                 _                      '
echo ' |  \/  |_   _| | |_(_) | ___   __ _  __| |         _ __   __ _ '
echo ' | |\/| | | | | | __| | |/ _ \ / _` |/ _` | ______ | `_ \ / _` |'
echo ' | |  | | |_| | | |_| | | (_) | (_| | (_| ||______|| | | | (_| |'
echo ' |_|  |_|\__,_|_|\__|_|_|\___/ \__,_|\__,_|        |_| |_|\__, |'
echo '                                                          |___/ '
echo
echo " ===================="
echo "  Build configuration"
echo " ===================="
echo " "
echo " - Link Time Optimization      (--enable-lto)   :  $enable_lto"
echo " - Debug-friendly build        (--enable-debug) :  $enable_debug"
echo " "
echo
