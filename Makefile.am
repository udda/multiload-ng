if HAVE_XFCE4
XFCE4_DIRS = xfce4
endif
if HAVE_LXPANEL
LXPANEL_DIRS = lxpanel
endif
SUBDIRS =	\
	multiload \
	$(XFCE4_DIRS) \
	$(LXPANEL_DIRS) \
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
	about-data-authors.h

about-data-authors.h: AUTHORS
	$(top_srcdir)/generate-about-data-authors.sh > about-data-authors.h

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
