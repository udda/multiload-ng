AC_ARG_WITH([systray], AS_HELP_STRING([--with-systray], [Build plugin for system tray icon]), [], [with_systray="yes"])

AC_DEFINE([HAVE_SYSTRAY], [1], [Support for system tray])

AC_CONFIG_FILES([
	systray/Makefile
	systray/multiload-ng-systray.desktop.in
])

AM_CONDITIONAL(HAVE_SYSTRAY, test "x$with_systray" = "xyes")
