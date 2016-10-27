AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
bin_PROGRAMS = multiload-ng-indicator

multiload_ng_indicator_SOURCES = plugin.c

multiload_ng_indicator_CFLAGS = \
	$(GTK_CFLAGS) \
	$(APPINDICATOR_CFLAGS)


multiload_ng_indicator_LDFLAGS = \
       -avoid-version \
       -no-undefined \
       $(PLATFORM_LDFLAGS)

multiload_ng_indicator_LDADD = \
	../common/multiload-ng-core.la \
	$(GTK_LIBS) \
	$(APPINDICATOR_LIBS)


# Desktop file
desktopdir = $(datadir)/applications
desktop_DATA = multiload-ng-indicator.desktop

@INTLTOOL_DESKTOP_RULE@

# Autostart
if ENABLE_AUTOSTART
autostartdir = $(sysconfdir)/xdg/autostart
autostart_DATA = $(desktop_DATA)
endif

EXTRA_DIST = multiload-ng-indicator.desktop.in.in
CLEANFILES = $(desktop_DATA)
DISTCLEANFILES = multiload-ng-indicator.desktop.in

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
