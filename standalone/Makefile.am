AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"standalone-multiload-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
bin_PROGRAMS = standalone-multiload

standalone_multiload_SOURCES = \
	standalone-multiload-plugin.c

standalone_multiload_CFLAGS = $(GTK_CFLAGS)

standalone_multiload_LDFLAGS = \
       -avoid-version \
       -no-undefined \
       $(PLATFORM_LDFLAGS)

standalone_multiload_LDADD = \
	$(GTK_LIBS) \
	-lm \
	../common/multiload-core.la

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
