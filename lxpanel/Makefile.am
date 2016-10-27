AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
plugin_LTLIBRARIES = \
	multiload_ng.la

plugindir = \
	$(libdir)/lxpanel/plugins

multiload_ng_la_SOURCES = \
	plugin.c plugin-old.c

multiload_ng_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(LXPANEL_CFLAGS)


multiload_ng_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       $(PLATFORM_LDFLAGS)

multiload_ng_la_LIBADD = \
	../common/multiload-ng-core.la \
	$(GTK_LIBS) \
	$(LXPANEL_LIBS)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
