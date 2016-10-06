# Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
#
# This file is part of multiload-ng.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#

# note: name of this makefile is lowercase, in order to not get ignored by .gitignore

CHECKINSTALL_FLAGS=\
	-y \
	--install=no \
	--nodoc \
	--pkglicense=GPL2 \
	--maintainer=mr.udda@gmail.com \
	--spec=extras/checkinstall/.spec \
	--pakdir=extras/checkinstall

all:
	@echo "Please select a target ( deb-package | slack-package | rpm-package )."

.PHONY: notice
notice:
	@echo "Note: checkinstall requires su privileges."

.PHONY: checkbuild
checkbuild:
	@cd ../.. ; if test ! -f common/multiload-ng-core.la ; then \
		echo "*** ERROR: configure and build package according to desired options before calling this."; \
		exit 1 ; fi

deb-package: checkbuild notice
	@cd ../.. ; checkinstall $(CHECKINSTALL_FLAGS) --type=debian

slack-package: checkbuild notice
	@cd ../.. ; checkinstall $(CHECKINSTALL_FLAGS) --type=slackware

rpm-package: checkbuild notice
	@cd ../.. ; checkinstall $(CHECKINSTALL_FLAGS) --type=rpm
