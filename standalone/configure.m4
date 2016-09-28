
AC_ARG_WITH([standalone], AS_HELP_STRING([--with-standalone], [Build standalone application]), [], [with_standalone=yes])

AC_DEFINE([HAVE_STANDALONE], [1], [Support for standalone])

AM_CONDITIONAL(HAVE_STANDALONE, test x$with_standalone != xno)

AC_CONFIG_FILES([
	standalone/Makefile
	standalone/multiload-ng-standalone.desktop.in
])
