AC_PREREQ([2.69])
AC_INIT([libmultiload], [2.0.0], [mr.udda@gmail.com], [libmultiload], [https://github.com/udda/multiload-ng])

# Store configure command line in config.h
configure_flags="$*"
AC_DEFINE_UNQUOTED([CONFIGURE_FLAGS],["$configure_flags"],[Flags passed to configure])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/multiload.h])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_SILENT_RULES(yes)

AC_COPYRIGHT([Copyright © 2017 Mario Cianciolo])

# Checks for programs.
AC_PROG_CC
PKG_PROG_PKG_CONFIG
AC_PATH_PROG([DOXYGEN], [doxygen], [no])


# Checks for libraries.
#AX_PKG_CHECK_MODULES([LIBPNG], [], [libpng >= 1.2.50])
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


# Conditional providers
#AC_ARG_ENABLE([javascript], AS_HELP_STRING([--enable-javascript], [Enable Javascript support]), [], [enable_javascript="yes"])
#AS_IF([test "x$enable_javascript" != "xno"], AC_DEFINE([ENABLE_ML_JAVASCRIPT], [1], [Enable Javascript support]))

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
