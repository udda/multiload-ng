AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"multiload-ng-systray\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
bin_PROGRAMS = multiload-ng-systray

multiload_ng_systray_SOURCES = plugin.c

multiload_ng_systray_CFLAGS = $(GTK_CFLAGS)

multiload_ng_systray_LDFLAGS = \
       -avoid-version \
       -no-undefined \
       $(PLATFORM_LDFLAGS)

multiload_ng_systray_LDADD = \
	../common/multiload-ng-core.la \
	$(GTK_LIBS)


# Desktop file
desktopdir = $(datadir)/applications
desktop_DATA = multiload-ng-systray.desktop

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = multiload-ng-systray.desktop.in.in
CLEANFILES = $(desktop_DATA)
DISTCLEANFILES = multiload-ng-systray.desktop.in

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
