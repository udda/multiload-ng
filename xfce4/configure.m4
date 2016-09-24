
AC_ARG_WITH([xfce4], AS_HELP_STRING([--with-xfce4], [Build plugin for xfce4-panel]), [], [with_xfce4=check])
AS_IF([test "x$with_xfce4" != "xno"], [

	# Select package versions depending on GTK version
	AS_IF([test "x$GTK_API" = "x2"], [
		XFCE_API=1.0
		XFCE_MIN_VERSION=4.6.0
	], [
		XFCE_API=2.0
		XFCE_MIN_VERSION=4.12.0
	])

	# Special case: check for XFCE 4.6 (different paths used)
	PKG_CHECK_EXISTS( [libxfce4ui-1 >= 4.8.0], [
		xfce_is_4_6=no
	], [
		PKG_CHECK_EXISTS( [libxfcegui4-1.0 >= 4.6.0], [
			xfce_is_4_6=yes
		])
	])

	# Check for packages
	PKG_CHECK_MODULES(XFCE4, [libxfce4util-1.0 >= $XFCE_MIN_VERSION libxfce4panel-$XFCE_API >= $XFCE_MIN_VERSION], [
		AC_DEFINE([HAVE_XFCE4], [1], [Support for xfce4-panel (GTK+$GTK_API)])
		with_xfce4=yes
	],[
		if test "x$with_xfce4" != xcheck; then
			AC_MSG_FAILURE([--with-xfce4 was given, but test failed])
		fi
		with_xfce4=no
	])
])

AM_CONDITIONAL([HAVE_XFCE4], [test x$with_xfce4 = xyes])
AM_CONDITIONAL([XFCE_IS_4_6], [test x$xfce_is_4_6 = xyes])

AC_SUBST(XFCE4_CFLAGS)
AC_SUBST(XFCE4_LIBS)
AC_SUBST(XFCE_API)

AC_CONFIG_FILES([
	xfce4/Makefile
	xfce4/multiload-ng-xfce4.desktop.in
])
