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
	binary-data.c binary-data.h \
	colors.c colors.h \
	colors-compat.c \
	color-scheme-icons.inc \
	filter.c filter.h \
	gtk-compat.c gtk-compat.h \
	graph-bat.c graph-cpu.c graph-mem.c graph-net.c graph-swap.c \
	graph-load.c graph-disk.c graph-temp.c graph-parm.c \
	graph-data.h \
	load-graph.c load-graph.h \
	multiload.c multiload.h \
	multiload-config.c multiload-config.h \
	preferences.c preferences.h \
	ps-settings-impl-gkeyfile.inc \
	util.c util.h \
	ui.c ui.h

multiload_ng_core_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(CAIRO_CFLAGS)


multiload_ng_core_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       -lm \
       $(PLATFORM_LDFLAGS)

multiload_ng_core_la_LIBADD = \
	$(GTK_LIBS) \
	$(CAIRO_LIBS)


COLOR_SCHEME_ICONS = \
	$(top_srcdir)/data/color-scheme-default.xpm \
	$(top_srcdir)/data/color-scheme-tango.xpm \
	$(top_srcdir)/data/color-scheme-solarized-dark.xpm \
	$(top_srcdir)/data/color-scheme-solarized-light.xpm \
	$(top_srcdir)/data/color-scheme-fruity.xpm \
	$(top_srcdir)/data/color-scheme-colored-glass.xpm \
	$(top_srcdir)/data/color-scheme-ubuntu-ambiance.xpm \
	$(top_srcdir)/data/color-scheme-ubuntu-radiance.xpm \
	$(top_srcdir)/data/color-scheme-linux-mint.xpm \
	$(top_srcdir)/data/color-scheme-windows-metro.xpm \
	$(top_srcdir)/data/color-scheme-arc.xpm \
	$(top_srcdir)/data/color-scheme-numix-dark.xpm \
	$(top_srcdir)/data/color-scheme-numix-light.xpm \
	$(top_srcdir)/data/color-scheme-moon.xpm \
	$(top_srcdir)/data/color-scheme-venus.xpm \
	$(top_srcdir)/data/color-scheme-earth.xpm \
	$(top_srcdir)/data/color-scheme-mars.xpm \
	$(top_srcdir)/data/color-scheme-jupiter.xpm \
	$(top_srcdir)/data/color-scheme-uranus.xpm \
	$(top_srcdir)/data/color-scheme-neptune.xpm

BUILT_SOURCES = \
	about-data.c \
	binary-data.c \
	color-scheme-icons.inc

about-data.c: $(top_srcdir)/AUTHORS $(top_srcdir)/data/generate-about-data.sh
	@chmod +x $(top_srcdir)/data/generate-about-data.sh
	( cd $(top_srcdir)/data; ./generate-about-data.sh $(top_srcdir)/AUTHORS ) > about-data.c

binary-data.c: $(top_srcdir)/data/preferences_gtk2.ui $(top_srcdir)/data/preferences_gtk3.ui $(top_srcdir)/data/generate-binary-data.sh
	@chmod +x $(top_srcdir)/data/generate-binary-data.sh
	( cd $(top_srcdir)/data; ./generate-binary-data.sh ) > binary-data.c

color-scheme-icons.inc: $(COLOR_SCHEME_ICONS) $(top_srcdir)/data/generate-color-scheme-icons.sh
	@chmod +x $(top_srcdir)/data/generate-color-scheme-icons.sh
	$(top_srcdir)/data/generate-color-scheme-icons.sh > color-scheme-icons.inc

CLEANFILES = \
	about-data.c \
	binary-data.c \
	color-scheme-icons.inc

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
