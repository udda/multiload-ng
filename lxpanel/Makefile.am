# INCLUDES =
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"lxpanel-multiload-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
plugin_LTLIBRARIES = \
	multiload.la

plugindir = \
	$(libdir)/lxpanel/plugins

multiload_la_SOURCES = \
	../common/about_data.h \
	../common/about_data-authors.h \
	lxpanel-multiload-plugin.c

multiload_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(LXPANEL_CFLAGS)

multiload_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       $(PLATFORM_LDFLAGS)

multiload_la_LIBADD = \
	$(GTK_LIBS) \
	$(LXPANEL_LIBS) \
	../common/multiload-core.la

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
