AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
plugin_LTLIBRARIES = \
	multiload-ng.la

plugindir = \
	$(libdir)/awn/applets/multiload-ng

multiload_ng_la_SOURCES = \
	plugin.c

multiload_ng_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(AWN_CFLAGS) \
	-std=gnu90


multiload_ng_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       $(PLATFORM_LDFLAGS)

multiload_ng_la_LIBADD = \
	../common/multiload-ng-core.la \
	$(GTK_LIBS) \
	$(AWN_LIBS)



# Schema
schemadir = $(datadir)/avant-window-navigator/schemas
schema_DATA = awn-applet-multiload-ng.schema-ini

# Desktop file
desktopdir = $(datadir)/avant-window-navigator/applets
desktop_DATA = multiload-ng-awn.desktop

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = $(desktop_DATA).in.in
CLEANFILES = $(desktop_DATA)
DISTCLEANFILES = $(desktop_DATA).in

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
