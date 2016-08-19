AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"multiload-ng-lxpanel\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
plugin_LTLIBRARIES = \
	multiload-ng.la

plugindir = \
	$(libdir)/lxpanel/plugins

multiload_ng_la_SOURCES = \
	plugin.c

multiload_ng_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(LXPANEL_CFLAGS)


multiload_ng_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       $(PLATFORM_LDFLAGS)

multiload_ng_la_LIBADD = \
	$(GTK_LIBS) \
	$(LXPANEL_LIBS) \
	../common/multiload-ng-core.la

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
