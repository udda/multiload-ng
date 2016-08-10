AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"multiload-ng-core\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
noinst_LTLIBRARIES = \
	multiload-ng-core.la

multiload_ng_core_la_SOURCES = \
	about-data.c about-data.h \
	autoscaler.c autoscaler.h \
	linux-proc.c linux-proc.h \
	load-graph.c load-graph.h \
	multiload.c multiload.h \
	multiload-colors.c multiload-colors.h \
	multiload-config.c multiload-config.h \
	properties.c properties.h \
	util.c util.h \
	ui.c ui.h

multiload_ng_core_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(LIBGTOP_CFLAGS)

# TESTING CFLAGS: uncomment to enable debugging (gdb) and profiling (gprof)
#multiload_ng_core_la_CFLAGS += -pg -ggdb -O0

multiload_ng_core_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       -lm \
       $(PLATFORM_LDFLAGS)

multiload_ng_core_la_LIBADD = \
	$(GTK_LIBS) \
	$(CAIRO_LIBS) \
	$(LIBGTOP_LIBS)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
