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


struct _MlConfig {
	MlAssocArray *aa;

	/* NOTE: MlConfig API is NOT thread-safe. This is usually not a problem, as each
	 * MlConfig is associated with only one MlGraph, so is always accessed by
	 * one thread at a time. The following variable stores formatted output
	 * from ml_config_get() */
	MlGrowBuffer *output;
};


// inner object: MlConfigEntry

typedef struct {
	MlValueType		type;
	mlPointer			address;

	char *label;
	char *description;

	bool bounds_set;
	int64_t bounds[2];
} MlConfigEntry;


static MlConfigEntry *
ml_config_entry_new (MlValueType type, mlPointer address, const char *label, const char *description)
{
	MlConfigEntry *entry = ml_new (MlConfigEntry);
	entry->type = type;
	entry->address = address;
	entry->bounds_set = false;
	entry->label = ml_strdup (label);
	entry->description = ml_strdup (description);

	return entry;
}

static void
ml_config_entry_destroy (MlConfigEntry *entry)
{
	if_unlikely (entry == NULL)
		return;

	free (entry->label);
	free (entry->description);
	free (entry);
}

static size_t
ml_config_entry_sizeof (MlConfigEntry *entry)
{
	if_unlikely (entry == NULL)
		return 0;

	size_t size = sizeof (MlConfigEntry);

	size += ml_string_sizeof (entry->label);
	size += ml_string_sizeof (entry->description);

	return size;
}



MlConfig *
ml_config_new ()
{
	MlConfig *config = ml_new (MlConfig);

	config->aa = ml_assoc_array_new (3, (MlDestroyFunc)ml_config_entry_destroy);
	config->output = ml_grow_buffer_new (16);

	return config;
}

void
ml_config_destroy (MlConfig *config)
{
	if_unlikely (config == NULL || config->aa == NULL)
		return;

	ml_assoc_array_destroy (config->aa);
	ml_grow_buffer_destroy (config->output, true);

	free (config);
}

size_t
ml_config_sizeof (MlConfig *config)
{
	if_unlikely (config == NULL || config->aa == NULL)
		return 0;

	size_t size = sizeof (MlConfig);
	size += ml_assoc_array_sizeof_with_fn (config->aa, (MlSizeofFunc)ml_config_entry_sizeof);
	size += ml_grow_buffer_sizeof (config->output);

	return size;
}

void
ml_config_add_entry (MlConfig *config, char *key, MlValueType type, mlPointer address, const char *label, const char *description)
{
	if_unlikely (config == NULL || config->aa == NULL)
		return;

	if (ml_assoc_array_get (config->aa, key) != NULL)
		ml_info ("Key '%s' already exists. New address/type will overwrite old one.", key);

	MlConfigEntry *entry = ml_config_entry_new (type, address, label, description);
	ml_assoc_array_put (config->aa, key, entry);
}

void
ml_config_add_entry_with_bounds (MlConfig *config, char *key, MlValueType type, mlPointer address, const char *label, const char *description, int64_t min, int64_t max)
{
	ml_config_add_entry (config, key, type, address, label, description);
	ml_config_set_entry_bounds (config, key, min, max);
}

MlValueType
ml_config_get_entry_type (MlConfig *config, const char *key)
{
	if_unlikely (config == NULL || config->aa == NULL)
		return ML_INVALID;

	if (key == NULL)
		return ML_INVALID;

	MlConfigEntry *entry = (MlConfigEntry*)ml_assoc_array_get (config->aa, key);
	if (entry == NULL) {
		ml_warning ("Key '%s' does not exist", key);
		return ML_INVALID;
	}

	return entry->type;
}

const char *
ml_config_get_label (MlConfig *config, const char *key)
{
	if_unlikely (config == NULL || config->aa == NULL)
		return NULL;

	if (key == NULL)
		return NULL;

	MlConfigEntry *entry = (MlConfigEntry*)ml_assoc_array_get (config->aa, key);
	if (entry == NULL) {
		ml_warning ("Key '%s' does not exist", key);
		return NULL;
	}

	return entry->label;
}

const char *
ml_config_get_description (MlConfig *config, const char *key)
{
	if_unlikely (config == NULL || config->aa == NULL)
		return NULL;

	if (key == NULL)
		return NULL;

	MlConfigEntry *entry = (MlConfigEntry*)ml_assoc_array_get (config->aa, key);
	if (entry == NULL) {
		ml_warning ("Key '%s' does not exist", key);
		return NULL;
	}

	return entry->description;
}

bool
ml_config_get_entry_bounds (MlConfig *config, const char *key, int64_t *min, int64_t *max)
{
	if_unlikely (config == NULL || config->aa == NULL)
		return false;

	if (key == NULL || min == NULL || max == NULL)
		return false;

	MlConfigEntry *entry = (MlConfigEntry*)ml_assoc_array_get (config->aa, key);
	if (entry == NULL) {
		ml_warning ("Key '%s' does not exist", key);
		return false;
	}

	if (!entry->bounds_set)
		return false;

	switch (entry->type) {
		case ML_VALUE_TYPE_INT32:
		case ML_VALUE_TYPE_INT64:
			*min = entry->bounds[0];
			*max = entry->bounds[1];
			break;

		default:
			ml_error ("Cannot get bounds for key '%s': wrong type", key);
			return false;
	}

	return true;
}

void
ml_config_set_entry_bounds (MlConfig *config, const char *key, int64_t min, int64_t max)
{
	if_unlikely (config == NULL || config->aa == NULL)
		return;

	if (key == NULL)
		return;

	MlConfigEntry *entry = (MlConfigEntry*)ml_assoc_array_get (config->aa, key);
	if (entry == NULL) {
		ml_warning ("Key '%s' does not exist", key);
		return;
	}

	switch (entry->type) {
		case ML_VALUE_TYPE_INT32: {
			if (min < INT32_MIN || max > INT32_MAX) {
				ml_error ("Specified bounds for key '%s' exceed the limits of underlying type (int32_t)", key);
				return;
			}

			entry->bounds[0] = min;
			entry->bounds[1] = max;
			break;
		}

		case ML_VALUE_TYPE_INT64: {
			entry->bounds[0] = min;
			entry->bounds[1] = max;
			break;
		}

		default: {
			ml_error ("Cannot set bounds for key '%s': wrong type", key);
			break;
		}
	}

	entry->bounds_set = true;
}

bool
ml_config_set (MlConfig *config, const char *key, const char *value)
{
	if_unlikely (config == NULL || config->aa == NULL)
		return false;

	if (key == NULL || value == NULL)
		return false;

	MlConfigEntry *entry = (MlConfigEntry*)ml_assoc_array_get (config->aa, key);
	if (entry == NULL) {
		ml_warning ("Key '%s' does not exist", key);
		return false;
	}

	char *endptr;

	switch (entry->type) {
		case ML_VALUE_TYPE_INT32: {
			int64_t v = (int64_t)ml_ascii_strtoll (value, &endptr, 0);
			if (value == endptr) {
				ml_error ("Failed to extract an integer from string '%s' for key '%s'", value, key);
				return false;
			}
			if (v > INT32_MAX || v < INT32_MIN) {
				ml_error ("Value for key '%s' exceeds the limits of the underlying type (int32_t): '%s'", key, value);
				return false;
			}
			if (entry->bounds_set && (v < entry->bounds[0] || v > entry->bounds[1])) {
				ml_error ("Value for key '%s' (%s) is out of bounds: [%"PRIi64", %"PRIi64"]", key, value, entry->bounds[0], entry->bounds[1]);
				return false;
			}
			ML_POINTER_LVALUE_AS(entry->address, int32_t) = (int32_t)v;
			return true;
		}

		case ML_VALUE_TYPE_INT64: {
			int64_t v = (int64_t)ml_ascii_strtoll (value, &endptr, 0);
			if (value == endptr) {
				ml_error ("Failed to extract an integer from string '%s' for key '%s'", value, key);
				return false;
			}
			if (entry->bounds_set && (v < entry->bounds[0] || v > entry->bounds[1])) {
				ml_error ("Value for key '%s' (%s) is out of bounds: [%"PRIi64", %"PRIi64"]", key, value, entry->bounds[0], entry->bounds[1]);
				return false;
			}
			ML_POINTER_LVALUE_AS(entry->address, int64_t) = v;
			return true;
		}

		case ML_VALUE_TYPE_TRISTATE: {
			// Every string that is not equal to "on" or "off" (case-insensitive) it treated as "auto"
			if (ml_string_equals (value, "on", false))
				ML_POINTER_LVALUE_AS(entry->address, MlTristate) = ML_ON;
			else if (ml_string_equals (value, "off", false))
				ML_POINTER_LVALUE_AS(entry->address, MlTristate) = ML_OFF;
			else
				ML_POINTER_LVALUE_AS(entry->address, MlTristate) = ML_AUTO;
			return true;
		}

		case ML_VALUE_TYPE_BOOLEAN: {
			// Every string that is not equal to "true" (case-insensitive) it treated as "false"
			ML_POINTER_LVALUE_AS(entry->address, bool) = ml_string_equals (value, "true", false);
			return true;
		}

		case ML_VALUE_TYPE_STRING: {
			free (ML_POINTER_VALUE_AS(entry->address, mlPointer));

			ML_POINTER_LVALUE_AS(entry->address, char*) = ml_strdup (value);
			return true;
		}

		case ML_VALUE_TYPE_STRV: {
			ml_strv_free (ML_POINTER_VALUE_AS(entry->address, char**));

			ML_POINTER_LVALUE_AS(entry->address, char**) = (value[0] == '\0') ? NULL : ml_string_split (value, ML_CONFIG_STRV_SEPARATOR);
			return true;
		}

		default:
			ml_error ("Unknown type (code %u) for key '%s'", (unsigned)entry->type, key);
			return false;
	}
}


const char*
ml_config_get (MlConfig *config, const char *key)
{
	if_unlikely (config == NULL || config->aa == NULL || config->output == NULL)
		return NULL;

	MlConfigEntry *entry = (MlConfigEntry*)ml_assoc_array_get (config->aa, key);
	if (entry == NULL) {
		ml_warning ("Key '%s' does not exist", key);
		return NULL;
	}

	switch (entry->type) {
		case ML_VALUE_TYPE_INT32: {
			ml_grow_buffer_rewind (config->output);
			ml_grow_buffer_append_printf (config->output, "%"PRIi32, ML_POINTER_VALUE_AS(entry->address, int32_t));
			return (const char *)ml_grow_buffer_get_data (config->output);
		}

		case ML_VALUE_TYPE_INT64: {
			ml_grow_buffer_append_printf (config->output, "%"PRIi64, ML_POINTER_VALUE_AS(entry->address, int64_t));
			return (const char *)ml_grow_buffer_get_data (config->output);
		}

		case ML_VALUE_TYPE_TRISTATE: {
			switch (ML_POINTER_VALUE_AS(entry->address, MlTristate)) {
				case ML_ON:
					return "on";
				case ML_OFF:
					return "off";
				case ML_AUTO:
				default:
					return "auto";
			}
		}

		case ML_VALUE_TYPE_BOOLEAN: {
			return ML_POINTER_VALUE_AS(entry->address, bool) ? "true" : "false";
		}

		case ML_VALUE_TYPE_STRING: {
			const char *v = (const char*)ML_POINTER_VALUE_AS(entry->address, char*);
			if (v == NULL)
				return "";

			return v;
		}

		case ML_VALUE_TYPE_STRV: {
			char **v = ML_POINTER_VALUE_AS(entry->address, char**);

			if (v == NULL)
				return "";

			ml_grow_buffer_rewind (config->output);
			ml_strv_join_to_grow_buffer (v, ML_CONFIG_STRV_SEPARATOR, config->output);
			return (const char *)ml_grow_buffer_get_data (config->output);

		}

		default:
			ml_error ("Unknown type (code %u) for key '%s'", (unsigned)entry->type, key);
			return NULL;
	}
}

cJSON *
ml_config_to_json (MlConfig *config)
{
	if_unlikely (config == NULL || config->aa == NULL)
		return NULL;

	char **keys = (char**)ml_assoc_array_get_keys (config->aa);
	cJSON* obj = cJSON_CreateObject ();

	ml_strv_for (keys, i) {
		const char *value = ml_config_get (config, keys[i]);
		cJSON_AddStringToObject (obj, keys[i], value);
	}

	free (keys);
	return obj;
}

void
ml_config_parse_json (MlConfig *config, cJSON *obj)
{
	if_unlikely (config == NULL || obj == NULL)
		return;

	cJSON *child = obj->child;
	if (child == NULL)
		return;

	do {
		ml_debug ("Set config entry \"%s\" => \"%s\"", child->string, child->valuestring);

		if (!ml_config_set (config, child->string, child->valuestring)) {
			ml_warning ("Cannot set config entry \"%s\" => \"%s\"", child->string, child->valuestring);
			continue;
		}

		child = child->next;
	} while (child != NULL);
}


/* IMPORTANT: albeit this is a char**, its elements belong to the associative
 * array. Only the outermost array must be freed (use free, not ml_strv_free) */
const char * const *
ml_config_list_keys (MlConfig *config)
{
	if_unlikely (config == NULL || config->aa == NULL)
		return NULL;

	return (const char * const *)ml_assoc_array_get_keys (config->aa);
}
