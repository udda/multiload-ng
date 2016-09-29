AS_IF([test "x$enable_experimental" = "xyes"], [

	AS_IF([test "x$GTK_API" = "x2"], [

		AC_ARG_WITH([awn], AS_HELP_STRING([--with-awn], [Build plugin for Avant Window Navigator]), [], [with_awn="check"])
		AS_IF([test "x$with_awn" != "xno"], [
			PKG_CHECK_MODULES(AWN, [awn >= 0.3.9 glibmm-2.4 >= 2.16.0 gtkmm-2.4], [
				AC_DEFINE([HAVE_AWN], [1], [Support for Avant Window Navigator])
				with_awn="yes"
			],[
				if test "x$with_awn" != xcheck; then
					AC_MSG_FAILURE([--with-awn was given, but test failed])
				fi
				with_awn="no"
			])
		])

		AC_SUBST(AWN_CFLAGS)
		AC_SUBST(AWN_LIBS)

		AC_CONFIG_FILES([
			awn/Makefile
			awn/multiload-ng-awn.desktop.in
		])

	], [
		with_awn="disabled (AWN does not support GTK+3 plugins)"
	])

], [
	with_awn="disabled (experimental)"
])

AM_CONDITIONAL(HAVE_AWN, test "x$with_awn" = "xyes")
