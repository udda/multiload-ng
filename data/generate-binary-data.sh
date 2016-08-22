#!/usr/bin/env bash
#
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


hexdump() {
	local BYTES_PER_LINE=16
	local A=0

	echo "const char $1[] = {"
	echo -n "    "

	IFS=

	while read -r -s -N 1 char; do
		printf "0x%02X, " "'$char"

		let A=A+1
		if [ "$A" = "$BYTES_PER_LINE" ]; then
			printf "\n    "
			A=0
		fi
	done
	echo "0x00"
	echo "};"
}

strdump() {
	echo 'const char *'$1' = '
	while read -r -s line; do
		echo '"'"${line//\"/\\\"}"'\n"'
	done
	echo ";"
}

echo "/* Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com> */"
echo "/* This file is part of multiload-ng. */"
echo
echo "/* THIS FILE IS GENERATED AUTOMATICALLY. DO NOT EDIT */"
echo
echo "#include <config.h>"
echo
echo
echo "#if GTK_API == 2"
echo
cat preferences_gtk2.ui | strdump "binary_data_preferences_ui"
echo
echo "#elif GTK_API == 3"
echo
cat preferences_gtk3.ui | strdump "binary_data_preferences_ui"
echo
echo "#else"
echo "#error Invalid GTK_API"
echo "#endif"
echo
