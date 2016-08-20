AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"multiload-ng-xfce\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
plugin_LTLIBRARIES = \
	libmultiload-ng.la

plugindir = \
	$(libdir)/xfce4/panel/plugins

libmultiload_ng_la_SOURCES = \
	plugin.c

libmultiload_ng_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(XFCE4_CFLAGS) $(XFCE4UI_CFLAGS) $(XFCEGUI4_CFLAGS) \
	-export-dynamic


libmultiload_ng_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       -export-symbols-regex '^xfce_panel_(module_(preinit|init|construct)|plugin_get_(check|construct))' \
       $(PLATFORM_LDFLAGS)

libmultiload_ng_la_LIBADD = \
	../common/multiload-ng-core.la \
	$(GTK_LIBS) \
	$(XFCE4_LIBS) $(XFCE4UI_LIBS) $(XFCEGUI4_LIBS)


#
# Desktop file
#
if XFCE_IS_4_6
desktopdir = $(datadir)/xfce4/panel-plugins
else
desktopdir = $(datadir)/xfce4/panel/plugins
endif

desktop_DATA = \
	multiload-ng.desktop

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	multiload-ng.desktop.in.in

CLEANFILES = \
	$(desktop_DATA)

DISTCLEANFILES = \
	multiload-ng.desktop.in

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
