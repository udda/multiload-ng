AS_IF([test "x$enable_experimental" = "xyes"], [

AC_ARG_WITH([awn], AS_HELP_STRING([--with-awn], [Build plugin for Avant Window Navigator]), [], [with_awn=check])
AS_IF([test "x$with_awn" != "xno"], [
	PKG_CHECK_MODULES(AWN, [awn >= 0.3.9], [
		AC_DEFINE([HAVE_AWN], [1], [Support for Avant Window Navigator])
		with_awn=yes
	],[
		if test "x$with_awn" != xcheck; then
			AC_MSG_FAILURE([--with-awn was given, but test failed])
		fi
		with_awn=no
	])
])

AM_CONDITIONAL(HAVE_AWN, test x$with_awn = xyes)

AC_SUBST(AWN_CFLAGS)
AC_SUBST(AWN_LIBS)

AC_CONFIG_FILES([
	awn/Makefile
	awn/multiload-ng-awn.desktop.in
])


], [
	with_awn="disabled (experimental)"
	AM_CONDITIONAL(HAVE_AWN, test "x$with_awn" = "xyes")
])
