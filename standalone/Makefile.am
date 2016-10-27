AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
bin_PROGRAMS = multiload-ng

multiload_ng_SOURCES = \
	plugin.c

multiload_ng_CFLAGS = \
	$(GTK_CFLAGS)


multiload_ng_LDFLAGS = \
       -avoid-version \
       -no-undefined \
       $(PLATFORM_LDFLAGS)

multiload_ng_LDADD = \
	../common/multiload-ng-core.la \
	$(GTK_LIBS)


# Desktop file
desktopdir = $(datadir)/applications
desktop_DATA = multiload-ng-standalone.desktop

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = multiload-ng-standalone.desktop.in.in
CLEANFILES = $(desktop_DATA)
DISTCLEANFILES = multiload-ng-standalone.desktop.in

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
