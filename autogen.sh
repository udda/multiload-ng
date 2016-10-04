#!/usr/bin/env bash

# Doesn't work with intltoolize
# autoreconf --force --install || exit 1

set -x

aclocal || exit 1
autoheader --force || exit 1
libtoolize --copy --automake --force || exit 1
intltoolize --copy --automake --force || exit 1
automake --add-missing --copy --include-deps || exit 1
autoconf || exit 1
autoheader || exit 1

