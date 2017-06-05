/*
 * Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of Multiload-ng.
 *
 * Multiload-ng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Multiload-ng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <multiload.h>


#define PATH_UPTIME "/proc/uptime"

double
ml_sys_info_get_uptime ()
{
	double uptime;
	if (ml_infofile_read_double (PATH_UPTIME, &uptime, 1))
		return uptime;
	else
		return 0;
}

const char*
ml_sys_info_get_kernel_name ()
{
	static char *buf = NULL;

	if_unlikely (buf == NULL) {
		struct utsname un;
		if (0 == uname(&un))
			buf = ml_strdup_printf ("%s %s (%s)", un.sysname, un.release, un.machine);
		else
			ml_warning ("uname() failed: could not get kernel name and version");
	}

	return (const char*)buf;
}

const char *
ml_sys_info_get_os_name ()
{
	static char *buf = NULL;

	if_unlikely (buf == NULL) {
		if (!ml_infofile_read_string ("/etc/oracle-release", &buf, NULL)
		&& !ml_infofile_read_string ("/etc/redhat-release", &buf, NULL)
		&& !ml_infofile_read_string ("/etc/SuSE-release", &buf, NULL)
		&& !ml_infofile_read_string ("/etc/lsb_release", &buf, NULL))
			ml_warning ("Could not get OS name");
	}
	return (const char*)buf;
}

int
ml_sys_info_get_page_size ()
{
	static int page_size = 0;
	if (page_size != 0)
		return page_size;

	page_size = sysconf (_SC_PAGESIZE);

	if (page_size == 0)
		page_size = 4096; // fallback

	return page_size;
}

MlGrowBuffer *
ml_sys_info_generate_sysfs_report (const char *sysfs_path)
{
	if_unlikely (sysfs_path == NULL)
		return NULL;

	MlGrowBuffer *gbuf = ml_grow_buffer_new(20480);

	ml_grow_buffer_append_printf (gbuf, "Listing sysfs entries in %s\n\n", sysfs_path);

	// calculate path length (include trailing backslash)
	size_t sysfs_path_len;
	if (ml_string_has_prefix (sysfs_path, "/sys"))
		sysfs_path_len = 5;
	else if (ml_string_has_prefix (sysfs_path, "/proc/sys"))
		sysfs_path_len = 10;
	else {
		sysfs_path_len = strlen (sysfs_path);
		if (!ml_string_has_suffix (sysfs_path, "/"))
			sysfs_path_len ++;
	}

	void _append (const char *path) {
		char key[PATH_MAX];

		ml_string_replace_all (path + sysfs_path_len, key, '/', '.');

		FILE *f = fopen (path, "r");
		if_unlikely (f == NULL) {
			ml_grow_buffer_append_printf (gbuf, "%s : access denied on this key (%s)\n", key, strerror (errno));
		} else {
			char buf[512];
			buf[0] = '\0';

			while (fgets (buf, sizeof(buf), f) != NULL) {
				size_t l = strlen (buf);
				if (buf[l-1] == '\n')
					buf[l-1] = '\0';

				ml_grow_buffer_append_printf (gbuf, "%s = %s\n", key, buf);
			}

			fclose (f);
		}
	}

	void _explore (const char *path, bool follow_links_levels) {
		if_unlikely (path == NULL)
			return;

		DIR *dir = opendir (path);
		if (dir == NULL) {
			ml_grow_buffer_append_printf (gbuf, "%s : access denied on this path\n", path);
			return;
		}

		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL) {
			if (ml_string_equals(ent->d_name, ".", false) || ml_string_equals(ent->d_name, "..", false))
				continue;

			char subpath[PATH_MAX];
			snprintf (subpath, sizeof(subpath), "%s/%s", path, ent->d_name);

			switch (ent->d_type) {
				case DT_DIR:
					_explore (subpath, follow_links_levels);
					break;
				case DT_REG:
					_append (subpath);
					break;
				case DT_LNK:
					if (follow_links_levels > 0) {
						struct stat st;

						if (stat (subpath, &st) < 0) {
							ml_grow_buffer_append_printf (gbuf, "%s : cannot resolve symbolic link\n", subpath);
						} else {
							if (S_ISREG(st.st_mode))
								_append (subpath);
							else if (S_ISDIR(st.st_mode))
								_explore (subpath, follow_links_levels-1);
						}
					} else {
						ml_grow_buffer_append_printf (gbuf, "%s : not following symbolic link\n", subpath);
					}
					break;
				default:
					break;
			}
		}

		closedir (dir);

	}

	_explore (sysfs_path, 1);

	return gbuf;
}

MlGrowBuffer *
ml_sys_info_generate_system_report ()
{
	char buf[200];

	MlGrowBuffer *gbuf = ml_grow_buffer_new(10240);

	// kernel
	const char *os_name = ml_sys_info_get_os_name();
	if (os_name != NULL)
		ml_grow_buffer_append_printf (gbuf, "Operating system:  %s\n", os_name);
	else
		ml_grow_buffer_append_printf (gbuf, "Error: cannot retrieve OS name\n");

	// kernel
	const char *kernel_name = ml_sys_info_get_kernel_name();
	if (kernel_name != NULL)
		ml_grow_buffer_append_printf (gbuf, "Kernel:            %s\n", kernel_name);
	else
		ml_grow_buffer_append_printf (gbuf, "Error: cannot retrieve kernel name and version\n");

	// uptime
	uint64_t uptime_seconds = (uint64_t)ml_sys_info_get_uptime();
	if (uptime_seconds > 0) {
		ml_string_format_time_s (uptime_seconds, buf, sizeof(buf));
		ml_grow_buffer_append_printf (gbuf, "Uptime:            %s\n", buf);
	} else {
		ml_grow_buffer_append_printf (gbuf, "Error: cannot retrieve system uptime\n");
	}

	ml_grow_buffer_append_printf (gbuf, "Current locale:    %s\n", setlocale (LC_ALL, NULL));
	return gbuf;
}

MlGrowBuffer *
ml_sys_info_generate_multiload_report ()
{
	MlGrowBuffer *gbuf = ml_grow_buffer_new(10240);
	ml_grow_buffer_append_printf (gbuf, "libmultiload\n------------\n");
	ml_grow_buffer_append_printf (gbuf, "Version:           %s\n", PACKAGE_VERSION);
	ml_grow_buffer_append_printf (gbuf, "Configure flags:   %s\n", CONFIGURE_FLAGS);

	ml_grow_buffer_append_printf (gbuf, "\nThird party software\n--------------------\n");
	ml_grow_buffer_append_printf (gbuf, "cJSON version:     %d.%d.%d\n", CJSON_VERSION_MAJOR, CJSON_VERSION_MINOR, CJSON_VERSION_PATCH );
	ml_grow_buffer_append_printf (gbuf, "Duktape version:   %d.%d.%d\n", DUK_VERSION / 10000, (DUK_VERSION%10000)/100, DUK_VERSION%100 );
	ml_grow_buffer_append_printf (gbuf, "Miniz version:     %s\n", MZ_VERSION);

	ml_grow_buffer_append_printf (gbuf, "\nShared configuration\n--------------------\n");
	cJSON *obj = ml_config_to_json (ML_SHARED_GET(config));
	char * json_config = cJSON_Print(obj);
	ml_grow_buffer_append_printf (gbuf, "%s\n", json_config);
	free (json_config);
	cJSON_Delete (obj);

	return gbuf;
}
