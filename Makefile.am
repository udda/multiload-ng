if HAVE_XFCE4
XFCE4_DIRS = xfce4
endif
if HAVE_LXPANEL
LXPANEL_DIRS = lxpanel
endif
if HAVE_MATE
MATE_DIRS = mate
endif
if HAVE_STANDALONE
STANDALONE_DIRS = standalone
endif


SUBDIRS =	\
	common \
	$(XFCE4_DIRS) \
	$(LXPANEL_DIRS) \
	$(MATE_DIRS) \
	$(STANDALONE_DIRS) \
	po

distclean-local:
	rm -rf *.cache *~

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

.PHONY: ChangeLog

ChangeLog: Makefile
	(GIT_DIR=$(top_srcdir)/.git git log > .changelog.tmp \
	&& mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) \
	|| (touch ChangeLog; echo 'Git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog

BUILT_SOURCES = \
	common/about-data.c

common/about-data.c: AUTHORS $(top_srcdir)/generate-about-data.sh
	@chmod +x $(top_srcdir)/generate-about-data.sh
	$(top_srcdir)/generate-about-data.sh > common/about-data.c

EXTRA_DIST = \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	AUTHORS COPYING ChangeLog README.md

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
