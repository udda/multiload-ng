AC_ARG_WITH([indicator], AS_HELP_STRING([--with-indicator], [Build plugin for libindicator]), [], [with_indicator=check])
AS_IF([test "x$with_indicator" != "xno"], [
	AS_IF([test "x$GTK_API" = "x2"], [LIBNAME=appindicator], [LIBNAME=appindicator3])

	PKG_CHECK_MODULES(APPINDICATOR, [$LIBNAME-0.1 >= 0.4.92], [
		AC_DEFINE([HAVE_APPINDICATOR], [1], [Support for libindicator])
		with_indicator=yes
	],[
		if test "x$with_indicator" != xcheck; then
			AC_MSG_FAILURE([--with-indicator was given, but test failed])
		fi
		with_indicator=no
	])
])

AM_CONDITIONAL(HAVE_APPINDICATOR, test x$with_indicator = xyes)

AC_SUBST(APPINDICATOR_CFLAGS)
AC_SUBST(APPINDICATOR_LIBS)

AC_CONFIG_FILES([
	indicator/Makefile
])
