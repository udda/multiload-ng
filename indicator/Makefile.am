AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"multiload-ng-indicator\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
bin_PROGRAMS = indicator-multiload-ng

indicator_multiload_ng_SOURCES = plugin.c

indicator_multiload_ng_CFLAGS = \
	$(GTK_CFLAGS) \
	$(APPINDICATOR_CFLAGS)


indicator_multiload_ng_LDFLAGS = \
       -avoid-version \
       -no-undefined \
       $(PLATFORM_LDFLAGS)

indicator_multiload_ng_LDADD = \
	../common/multiload-ng-core.la \
	$(GTK_LIBS) \
	$(APPINDICATOR_LIBS)


#
# Desktop file
#
#desktopdir = $(datadir)/applications

#desktop_DATA = indicator-multiload-ng.desktop

#@INTLTOOL_DESKTOP_RULE@

#EXTRA_DIST = indicator-multiload-ng.desktop.in.in

#CLEANFILES = $(desktop_DATA)

#DISTCLEANFILES = indicator-multiload-ng.desktop.in

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
