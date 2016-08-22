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
	binary-data.c binary-data.h \
	autoscaler.c autoscaler.h \
	gtk-compat.c gtk-compat.h \
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



BUILT_SOURCES = \
	about-data.c \
	binary-data.c

about-data.c: $(top_srcdir)/AUTHORS $(top_srcdir)/data/generate-about-data.sh
	@chmod +x $(top_srcdir)/data/generate-about-data.sh
	( cd $(top_srcdir)/data; ./generate-about-data.sh $(top_srcdir)/AUTHORS ) > about-data.c

binary-data.c: $(top_srcdir)/data/preferences_gtk2.ui $(top_srcdir)/data/preferences_gtk3.ui $(top_srcdir)/data/generate-binary-data.sh
	@chmod +x $(top_srcdir)/data/generate-binary-data.sh
	( cd $(top_srcdir)/data; ./generate-binary-data.sh ) > binary-data.c

CLEANFILES = \
	about-data.c \
	binary-data.c

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
