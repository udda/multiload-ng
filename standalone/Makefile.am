AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"multiload-ng-standalone\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
bin_PROGRAMS = multiload-ng

multiload_ng_SOURCES = \
	standalone-multiload-plugin.c

multiload_ng_CFLAGS = $(GTK_CFLAGS)

multiload_ng_LDFLAGS = \
       -avoid-version \
       -no-undefined \
       $(PLATFORM_LDFLAGS)

multiload_ng_LDADD = \
	$(GTK_LIBS) \
	-lm \
	../common/multiload-ng-core.la

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
