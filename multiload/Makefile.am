# INCLUDES =
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/multiload \
	-DG_LOG_DOMAIN=\"multiload-core\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
noinst_LTLIBRARIES = \
	multiload-core.la

multiload_core_la_SOURCES = \
	autoscaler.c autoscaler.h \
	linux-proc.c linux-proc.h \
	load-graph.c load-graph.h \
	multiload.c multiload.h \
	multiload-colors.c multiload-colors.h \
	multiload-config.c multiload-config.h \
	properties.c properties.h \
	util.c util.h

multiload_core_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(LIBGTOP_CFLAGS)

multiload_core_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       -export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
       $(PLATFORM_LDFLAGS)

multiload_core_la_LIBADD = \
	$(GTK_LIBS) \
	$(CAIRO_LIBS) \
	$(LIBGTOP_LIBS)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
