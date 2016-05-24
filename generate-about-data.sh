#!/bin/sh
echo "#include <glib.h>"
echo
echo "const gchar * const about_data_authors[] = {"
IFS=$'\n'
for line in $(cat AUTHORS); do
	echo "	\"$line\","
done
echo "	NULL"
echo "};"
