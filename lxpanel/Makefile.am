# INCLUDES =
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/multiload \
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
	../about_data.h \
	../about_data-authors.h \
	lxpanel-multiload-plugin.c

multiload_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(LXPANEL_CFLAGS)

multiload_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       $(PLATFORM_LDFLAGS)

# export-symbols-regex '^multiload_(constructor|destructor|configure|save_configuration|configuration_changed)'

multiload_la_LIBADD = \
	$(GTK_LIBS) \
	$(LXPANEL_LIBS) \
	../multiload/multiload-core.la

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
