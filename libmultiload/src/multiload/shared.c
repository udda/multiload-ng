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


// singleton
MlMultiloadShared *singleton = NULL;


/* This function allocates the shared resources needed by all Multiload-ng
 * instances, and performs initial environment configuration. It must be called
 * by every library entry point */
void
ml_multiload_shared_init ()
{
	if_unlikely (singleton != NULL)
		return; // already initialized


	// gettext stuff
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");


	// MlMultiloadShared stuff
	singleton = ml_new (MlMultiloadShared);

	singleton->C_locale = newlocale (LC_ALL_MASK, "C", NULL);
	singleton->procmon = ml_process_monitor_new ();
	singleton->dev_fstypes = ml_filesystem_info_list_dev_fstypes();

	singleton->debug_mask = ml_debug_get_mask ();
	singleton->debug_output = ml_debug_get_output_file ();

	singleton->config = ml_config_new ();
	singleton->console_colors = ML_AUTO;
	singleton->procmon_enable = true;


	ml_config_add_entry (singleton->config,
		"console_colors",
		ML_VALUE_TYPE_TRISTATE,
		&singleton->console_colors,
		_("Colored console output"),
		_("Use colored text when printing to terminal.")
	);

	ml_config_add_entry (singleton->config,
		"procmon_enable",
		ML_VALUE_TYPE_BOOLEAN,
		&singleton->procmon_enable,
		_("Enable process monitor"),
		_("Process monitor provides lists of processes ordered by usage of system resources.")
	);
}


// automatically called on library unload (tipically when program terminates)
static ML_FN_DESTRUCTOR void
ml_multiload_shared_destroy ()
{
	if_unlikely (singleton == NULL)
		return; // not initialized

	ml_info ("Goodbye!");

	freelocale (singleton->C_locale);
	ml_process_monitor_destroy (singleton->procmon);
	ml_strv_free (singleton->dev_fstypes);

	fclose (singleton->debug_output);

	ml_config_destroy (singleton->config);

	free (singleton);
}


const MlMultiloadShared *
ml_multiload_shared_get ()
{
	ml_multiload_shared_init ();
	return (const MlMultiloadShared *)singleton;
}
