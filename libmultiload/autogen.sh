#!/usr/bin/env bash

set -x

mkdir -p m4
autopoint --force || exit 1
aclocal || exit 1
libtoolize --force --copy || exit 1
autoheader --force || exit 1
automake --force --add-missing || exit 1
autoconf --force || exit 1
#autoheader || exit 1
