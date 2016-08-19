
AC_ARG_WITH([xfce4], AS_HELP_STRING([--with-xfce4], [Build plugin for xfce4-panel]), [], [with_xfce4=check])
AS_IF([test "x$with_xfce4" != "xno"], [
	PKG_CHECK_MODULES(XFCE4, [libxfce4util-1.0 >= 4.6.0 libxfce4panel-1.0 >= 4.6.0], [
		AC_DEFINE([HAVE_XFCE4], [1], [Support for xfce4-panel])
		with_xfce4=yes
		# Check if we have libxfce4ui or libxfcegui4
		PKG_CHECK_MODULES([XFCE4UI], [libxfce4ui-1 >= 4.8.0], [
			AC_DEFINE([HAVE_XFCE4UI], [1], [Have libxfce4ui])
			xfce_is_4_6=no
		],[
			PKG_CHECK_MODULES([XFCEGUI4], [libxfcegui4-1.0 >= 4.6.0], [AC_DEFINE([HAVE_XFCEGUI4], [1], [Have libxfcegui4])])
			xfce_is_4_6=yes
		])
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
AC_SUBST(XFCE4UI_CFLAGS)
AC_SUBST(XFCE4UI_LIBS)
AC_SUBST(XFCEGUI4_CFLAGS)
AC_SUBST(XFCEGUI4_LIBS)
