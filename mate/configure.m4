
AC_ARG_WITH([mate], AS_HELP_STRING([--with-mate], [Build plugin for mate-panel]), [], [with_mate=check])
AS_IF([test "x$with_mate" != "xno"], [
	GLIB_GSETTINGS
	PKG_CHECK_MODULES(MATE, [libmatepanelapplet-4.0 >= 1.7.0], [
		AC_DEFINE([HAVE_MATE], [1], [Support for mate-panel])
		with_mate=yes
	],[
		if test "x$with_mate" != xcheck; then
			AC_MSG_FAILURE([--with-mate was given, but test failed])
		fi
		with_mate=no
	])
])

AM_CONDITIONAL(HAVE_MATE, test x$with_mate = xyes)

AC_SUBST(MATE_CFLAGS)
AC_SUBST(MATE_LIBS)

AC_CONFIG_FILES([
	mate/Makefile
	mate/org.mate.multiload-ng.Applet.mate-panel-applet.in.in
	mate/org.mate.panel.applet.multiload-ng.gschema.xml.in
])
