AC_ARG_WITH([lxpanel], AS_HELP_STRING([--with-lxpanel], [Build plugin for lxpanel]), [], [with_lxpanel="check"])
AS_IF([test "x$with_lxpanel" != "xno"], [
	PKG_CHECK_MODULES(LXPANEL, [lxpanel >= 0.5.8], [
		# libmenu-cache is implicitly required
		AC_DEFINE([HAVE_LXPANEL], [1], [Support for lxpanel])
		with_lxpanel="yes"

		# Check for LxPanel>=0.7.0 (new API)
		PKG_CHECK_EXISTS( [lxpanel >= 0.7.0 libfm >= 1.2.0], [ AC_DEFINE([LXPANEL_NEW_API], [1], [LxPanel version is >= 0.7.0]) ])

	],[
		if test "x$with_lxpanel" != "xcheck"; then
			AC_MSG_FAILURE([--with-lxpanel was given, but test failed])
		fi
		with_lxpanel="no"
	])
])

AC_SUBST(LXPANEL_CFLAGS)
AC_SUBST(LXPANEL_LIBS)

AC_CONFIG_FILES([
	lxpanel/Makefile
])

AM_CONDITIONAL(HAVE_LXPANEL, test "x$with_lxpanel" = "xyes")
