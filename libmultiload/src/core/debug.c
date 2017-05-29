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


char **
ml_debug_get_backtrace (size_t *len_ptr)
{
	if (len_ptr == NULL)
		return NULL;

	mlPointer entries[ML_DEBUG_BACKTRACE_SIZE];

	*len_ptr = backtrace (entries, 10);
	return backtrace_symbols (entries, *len_ptr);
}

const char *
ml_debug_level_to_string (MlDebugLevel level)
{
	switch (level) {
		case ML_DEBUG_LEVEL_DEBUG:
			return "DEBUG";
		case ML_DEBUG_LEVEL_INFO:
			return "INFO";
		case ML_DEBUG_LEVEL_WARNING:
			return "WARNING";
		case ML_DEBUG_LEVEL_ERROR:
			return "ERROR";
		case ML_DEBUG_LEVEL_BUG:
			return "BUG";
		default:
			return "???";
	}
}

void
ml_debug_printf_full (MlDebugLevel level, const char *function, const char *file, int line, bool backtrace, const char *fmt, ...)
{
	// don't print disabled debug levels (bug messages are always shown)
	if (!(ML_SHARED_GET(debug_mask) & level) && (level != ML_DEBUG_LEVEL_BUG))
		return;

	char buf[ML_DEBUG_MAX_LINE_LENGTH];

	va_list va;
	va_start (va, fmt);
	vsnprintf (buf, sizeof(buf), fmt, va);
	va_end (va);

	// timestamp in current locale
	time_t time_now = time(NULL);
	struct tm *tm_now = localtime(&time_now);
	char timestamp[100];
	if (tm_now == NULL || strftime (timestamp, sizeof(timestamp), "[%b %d %T]", tm_now) == 0)
		timestamp[0] = '\0';

	// basename of source file
	char *basename = strrchr (file, '/');
	if (basename != NULL)
		basename++;
	else
		basename = (char*)file;

	// detect output file, with fallback to stderr
	FILE * out = ML_SHARED_GET (debug_output);
	if (out == NULL)
		out = stderr;

	// detect color support
	bool color_support;
	MlTristate tr = ML_SHARED_GET (console_colors);
	if (tr == ML_ON)
		color_support = true;
	else if (tr == ML_OFF)
		color_support = false;
	else { // if (tr == ML_AUTO)
		/* For autodetect of color support in current terminal, we rely on isatty function.
		 * Actually the function only checks whether output is redirected to a terminal
		 * or not, but it's pretty safe to assume that every modern console / terminal
		 * emulator supports ANSI color sequences */
		if (isatty(fileno(out)))
			color_support = true;
		else
			color_support = false;

	}


	// set colors based on message level
	MlAnsiColor bg_color;
	MlAnsiColor fg_color;
	switch (level) {
		case ML_DEBUG_LEVEL_DEBUG:
			bg_color = ML_ANSI_DEFAULT;
			fg_color = ML_ANSI_BLUE;
			break;
		case ML_DEBUG_LEVEL_INFO:
			bg_color = ML_ANSI_DEFAULT;
			fg_color = ML_ANSI_GREEN;
			break;
		case ML_DEBUG_LEVEL_WARNING:
			bg_color = ML_ANSI_DEFAULT;
			fg_color = ML_ANSI_YELLOW;
			break;
		case ML_DEBUG_LEVEL_ERROR:
			bg_color = ML_ANSI_DEFAULT;
			fg_color = ML_ANSI_MAGENTA;
			break;
		case ML_DEBUG_LEVEL_BUG:
			bg_color = ML_ANSI_DEFAULT;
			fg_color = ML_ANSI_RED;
			break;
		default:
			bg_color = ML_ANSI_DEFAULT;
			fg_color = ML_ANSI_DEFAULT;
			break;
	}


	// empty line
	fprintf (out, "\n");

	// header row
	if (color_support) ml_ansi_print_sequence (out, bg_color, fg_color, true);
	fprintf (out, "### [Multiload-ng]");
	if (color_support) ml_ansi_print_sequence (out, bg_color, fg_color, false);
	fprintf (out, "  %s (%s:%d)  %s", function, basename, line, timestamp);
	if (color_support) ml_ansi_print_sequence (out, ML_ANSI_DEFAULT, ML_ANSI_DEFAULT, false);
	fprintf (out, "\n");

	// message
	if (color_support) ml_ansi_print_sequence (out, bg_color, fg_color, true);
	fprintf (out, "%17s:", ml_debug_level_to_string (level));
	if (color_support) ml_ansi_print_sequence (out, ML_ANSI_DEFAULT, ML_ANSI_DEFAULT, false);
	// multiline print
	char *temp;
	char *pch = strtok_r (buf, "\n", &temp);
	fprintf (out, "  %s\n", pch);
	while ((pch = strtok_r(NULL, "\n", &temp)) != NULL)
		fprintf (out, "                    %s\n", pch);

	// backtrace
	if (backtrace) {
		size_t len;
		char ** entries = ml_debug_get_backtrace (&len);
		for (size_t i = 0; i < len; i++) {
			if (color_support) ml_ansi_print_sequence (out, bg_color, fg_color, true);
			fprintf (out, "%15s %2zu", (i==0?"BT":""), i);

			if (color_support) ml_ansi_print_sequence (out, ML_ANSI_DEFAULT, ML_ANSI_DEFAULT, false);
			fprintf (out, "  %s\n", entries[i]);
		}
		free (entries);
	}
}

int
ml_debug_get_mask ()
{
	const char *env_debug_level = getenv (ML_DEBUG_LEVEL_ENVIRON_KEY);
	if (env_debug_level == NULL)
		return ML_DEBUG_DEFAULT_LEVEL;

	char **levels = ml_string_split (env_debug_level, ",");
	int ret = 0;

	ml_strv_for (levels, i) {

		if (ml_string_equals (levels[i], "all", false)) {
			ret = 0xFFFFFFFF;
			break;
		}

		if (ml_string_equals (levels[i], "debug", false)) {
			ret |= ML_DEBUG_LEVEL_DEBUG;
			continue;
		}

		if (ml_string_equals (levels[i], "info", false)) {
			ret |= ML_DEBUG_LEVEL_INFO;
			continue;
		}

		if (ml_string_equals (levels[i], "warning", false)) {
			ret |= ML_DEBUG_LEVEL_WARNING;
			continue;
		}

		if (ml_string_equals (levels[i], "error", false)) {
			ret |= ML_DEBUG_LEVEL_ERROR;
			continue;
		}

	}

	ml_strv_free (levels);

	return ret;
}

FILE *
ml_debug_get_output_file ()
{
	/* This function returns a FILE to a writable path. In case of any error,
	 * the returned FILE will point to stderr */
	FILE *ret;

	const char *env_debug_path = getenv (ML_DEBUG_FILE_ENVIRON_KEY);

	if (env_debug_path == NULL) {
		ret = fdopen (STDERR_FILENO, "a");
	} else {
		ret = fopen (env_debug_path, "a");
		if (ret == NULL) {
			ret = fdopen (STDERR_FILENO, "a");
			ml_warning ("Cannot open '%s' for debug output: %s", env_debug_path, strerror (errno));
		}
	}

	if (ret == NULL)
		ml_bug ("Cannot create debug output FILE");

	return ret;
}

void
ml_debug_timespec_init (struct timespec *tm)
{
	clock_gettime (CLOCK_MONOTONIC_RAW, tm);
}

void
ml_debug_timespec_print_elapsed (struct timespec *tm, bool update)
{
	struct timespec end;

	clock_gettime (CLOCK_MONOTONIC_RAW, &end);

	ml_debug ("Processing time: %.06f msec\n", (double)(end.tv_sec - tm->tv_sec)*1000 + (double)(end.tv_nsec - tm->tv_nsec) / 1000000);

	if (update)
		memcpy (tm, &end, sizeof(struct timespec));
}


bool
ml_debug_collect_to_zip (const char *filename, int compression_level)
{
	if (compression_level < 0 || compression_level > 9)
		return false;

	mz_zip_archive zip;
	mz_zip_zero_struct(&zip);

	if (!mz_zip_writer_init_file(&zip, filename, 0))
		return false;

	const char *readme_text =
		"This ZIP file was generated by Multiload-ng.\n"
		"It contains data about your system that will be useful for troubleshooting.\n"
		"\n"
		"Data includes:\n"
		"- Multiload-ng version and configuration\n"
		"- Data about your hardware (disks, network interfaces, ...)\n"
		"- Data about your system (OS name, boot command line, loaded modules, ...)\n"
		"- Environment variables\n"
		"\n"
		"Data DOES NOT include:\n"
		"- Your name, any of your personal details (accounts, emails, passwords, ...)\n"
		"- Your personal files (documents, images, ...)\n"
		"- Your browser history or other personal habits\n"
		"\n"
		"You are free to explore this archive. Files inside are well organized,\n"
		"and they are as easily readable as possible.\n";
	ml_zip_add_string (&zip, "README", readme_text, compression_level);

	// procfs files
	ml_zip_add_file (&zip, "proc/cmdline",        "/proc/cmdline",        compression_level);
	ml_zip_add_file (&zip, "proc/config.gz",      "/proc/config.gz",      compression_level);
	ml_zip_add_file (&zip, "proc/cpuinfo",        "/proc/cpuinfo",        compression_level);
	ml_zip_add_file (&zip, "proc/devices",        "/proc/devices",        compression_level);
	ml_zip_add_file (&zip, "proc/diskstats",      "/proc/diskstats",      compression_level);
	ml_zip_add_file (&zip, "proc/filesystems",    "/proc/filesystems",    compression_level);
	ml_zip_add_file (&zip, "proc/meminfo",        "/proc/meminfo",        compression_level);
	ml_zip_add_file (&zip, "proc/modules",        "/proc/modules",        compression_level);
	ml_zip_add_file (&zip, "proc/mounts",         "/proc/mounts",         compression_level);
	ml_zip_add_file (&zip, "proc/net/dev",        "/proc/net/dev",        compression_level);
	ml_zip_add_file (&zip, "proc/net/sockstat",   "/proc/net/sockstat",   compression_level);
	ml_zip_add_file (&zip, "proc/net/sockstat6",  "/proc/net/sockstat6",  compression_level);
	ml_zip_add_file (&zip, "proc/net/wireless",   "/proc/net/wireless",   compression_level);
	ml_zip_add_file (&zip, "proc/partitions",     "/proc/partitions",     compression_level);
	ml_zip_add_file (&zip, "proc/stat",           "/proc/stat",           compression_level);
	ml_zip_add_file (&zip, "proc/swaps",          "/proc/swaps",          compression_level);
	ml_zip_add_file (&zip, "proc/uptime",         "/proc/uptime",         compression_level);
	ml_zip_add_file (&zip, "proc/version",        "/proc/version",        compression_level);
	ml_zip_add_file (&zip, "proc/vmstat",         "/proc/vmstat",         compression_level);


	// parsed data from Ml*Info
	MlGrowBuffer *gbuf_battery      = ml_battery_info_generate_report ();
	MlGrowBuffer *gbuf_cpu          = ml_cpu_info_generate_report ();
	MlGrowBuffer *gbuf_disk         = ml_disk_info_generate_report ();
	MlGrowBuffer *gbuf_filesystem   = ml_filesystem_info_generate_report ();
	MlGrowBuffer *gbuf_net          = ml_net_info_generate_report ();
	MlGrowBuffer *gbuf_temperature  = ml_temperature_info_generate_report ();
	MlGrowBuffer *gbuf_system       = ml_sys_info_generate_system_report ();

	ml_zip_add_grow_buffer(&zip, "report/battery.report",      gbuf_battery,      compression_level);
	ml_zip_add_grow_buffer(&zip, "report/cpu.report",          gbuf_cpu,          compression_level);
	ml_zip_add_grow_buffer(&zip, "report/disk.report",         gbuf_disk,         compression_level);
	ml_zip_add_grow_buffer(&zip, "report/filesystem.report",   gbuf_filesystem,   compression_level);
	ml_zip_add_grow_buffer(&zip, "report/net.report",          gbuf_net,          compression_level);
	ml_zip_add_grow_buffer(&zip, "report/temperature.report",  gbuf_temperature,  compression_level);
	ml_zip_add_grow_buffer(&zip, "report/system.report",       gbuf_system,       compression_level);

	ml_grow_buffer_destroy (gbuf_battery, true);
	ml_grow_buffer_destroy (gbuf_cpu, true);
	ml_grow_buffer_destroy (gbuf_disk, true);
	ml_grow_buffer_destroy (gbuf_filesystem, true);
	ml_grow_buffer_destroy (gbuf_net, true);
	ml_grow_buffer_destroy (gbuf_temperature, true);
	ml_grow_buffer_destroy (gbuf_system, true);


	// multiload info
	MlGrowBuffer *gbuf_multiload = ml_sys_info_generate_multiload_report ();
	ml_zip_add_grow_buffer(&zip, "multiload.info",  gbuf_multiload, compression_level);
	ml_grow_buffer_destroy (gbuf_multiload, true);


	// environment variables
	MlGrowArray *ga_environ = ml_grow_array_new_from_strv (environ, NULL, false);
	ml_grow_array_sort (ga_environ, (MlCompareFunc)ml_string_compare_func);

	MlGrowBuffer *gbuf_environ = ml_grow_buffer_new (1024);
	ml_grow_array_for (ga_environ, i)
		ml_grow_buffer_append_printf (gbuf_environ, "%s\n", (char*)ml_grow_array_get(ga_environ, i));

	ml_zip_add_grow_buffer(&zip, "system/environ", gbuf_environ, compression_level);

	ml_grow_array_destroy (ga_environ, true, false);
	ml_grow_buffer_destroy (gbuf_environ, true);


	// interesting data in /sys (whole directory is too much data)
	MlGrowBuffer *gbuf_sysfs_hwmon         = ml_sys_info_generate_sysfs_report ("/sys/class/hwmon");
	MlGrowBuffer *gbuf_sysfs_net           = ml_sys_info_generate_sysfs_report ("/sys/class/net");
	MlGrowBuffer *gbuf_sysfs_power_supply  = ml_sys_info_generate_sysfs_report ("/sys/class/power_supply");
	MlGrowBuffer *gbuf_sysfs_thermal       = ml_sys_info_generate_sysfs_report ("/sys/class/thermal");
	MlGrowBuffer *gbuf_sysfs_cpu           = ml_sys_info_generate_sysfs_report ("/sys/devices/system/cpu");
	// kernel parameters in /proc/sys
	MlGrowBuffer *gbuf_sysfs_proc          = ml_sys_info_generate_sysfs_report ("/proc/sys");

	ml_zip_add_grow_buffer (&zip, "sysfs/hwmon.sfs",        gbuf_sysfs_hwmon,        compression_level);
	ml_zip_add_grow_buffer (&zip, "sysfs/net.sfs",          gbuf_sysfs_net,          compression_level);
	ml_zip_add_grow_buffer (&zip, "sysfs/power_supply.sfs", gbuf_sysfs_power_supply, compression_level);
	ml_zip_add_grow_buffer (&zip, "sysfs/thermal.sfs",      gbuf_sysfs_thermal,      compression_level);
	ml_zip_add_grow_buffer (&zip, "sysfs/cpu.sfs",          gbuf_sysfs_cpu,          compression_level);
	ml_zip_add_grow_buffer (&zip, "sysfs/proc_sys.sfs",     gbuf_sysfs_proc,         compression_level);

	ml_grow_buffer_destroy (gbuf_sysfs_hwmon,        true);
	ml_grow_buffer_destroy (gbuf_sysfs_net,          true);
	ml_grow_buffer_destroy (gbuf_sysfs_power_supply, true);
	ml_grow_buffer_destroy (gbuf_sysfs_thermal,      true);
	ml_grow_buffer_destroy (gbuf_sysfs_cpu,          true);
	ml_grow_buffer_destroy (gbuf_sysfs_proc,         true);


	// output for some commands
	MlGrowBuffer *command_join (const char *command_line) {
		MlGrowBuffer *ret = ml_grow_buffer_new(100);

		MlGrowBuffer *gbuf_command_out = ml_grow_buffer_new (0);
		MlGrowBuffer *gbuf_command_err = ml_grow_buffer_new (0);
		MlGrowBuffer *gbuf_command_rep = ml_grow_buffer_new (0);

		ml_execute_cmdline_sync (command_line, NULL, -1, gbuf_command_out, gbuf_command_err, gbuf_command_rep, NULL);

		ml_grow_buffer_append_printf (ret, "Command line: %s\n", command_line);
		ml_grow_buffer_append_string (ret, "\n---- BEGIN STDOUT ----\n");
		ml_grow_buffer_append_grow_buffer (ret, gbuf_command_out);
		ml_grow_buffer_append_string (ret, "\n---- END STDOUT ----\n");
		ml_grow_buffer_append_string (ret, "\n---- BEGIN STDERR ----\n");
		ml_grow_buffer_append_grow_buffer (ret, gbuf_command_err);
		ml_grow_buffer_append_string (ret, "\n---- END STDERR ----\n");
		ml_grow_buffer_append_string (ret, "\n---- BEGIN REPORT ----\n");
		ml_grow_buffer_append_grow_buffer (ret, gbuf_command_rep);
		ml_grow_buffer_append_string (ret, "\n---- END REPORT ----\n");

		ml_grow_buffer_destroy (gbuf_command_out, true);
		ml_grow_buffer_destroy (gbuf_command_err, true);
		ml_grow_buffer_destroy (gbuf_command_rep, true);

		return ret;
	}

	MlGrowBuffer *gbuf_command_lsb_release  = command_join ("lsb_release -a");
	MlGrowBuffer *gbuf_command_lsproc       = command_join ("ls -las /proc/");
	MlGrowBuffer *gbuf_command_ping         = command_join ("ping -V");

	ml_zip_add_grow_buffer (&zip, "commands/lsb_release",  gbuf_command_lsb_release, compression_level);
	ml_zip_add_grow_buffer (&zip, "commands/ls_proc",      gbuf_command_lsproc, compression_level);
	ml_zip_add_grow_buffer (&zip, "commands/ping_v",       gbuf_command_ping, compression_level);

	ml_grow_buffer_destroy (gbuf_command_lsb_release,  true);
	ml_grow_buffer_destroy (gbuf_command_lsproc,       true);
	ml_grow_buffer_destroy (gbuf_command_ping,         true);


	if (!mz_zip_writer_finalize_archive(&zip))
		return false;

	if (!mz_zip_writer_end(&zip))
		return false;

	return true;
}
