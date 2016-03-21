#!/bin/sh

echo "const gchar * const about_data_authors[] = {"
IFS=$'\n'
for line in $(cat AUTHORS); do
	echo "	\"$line\","
done
echo "	NULL"
echo "};"
