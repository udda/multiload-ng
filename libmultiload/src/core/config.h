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

#ifndef ML_HEADER__GRAPH_CONFIG_H__INCLUDED
#define ML_HEADER__GRAPH_CONFIG_H__INCLUDED
ML_HEADER_BEGIN
#include "types.h"


typedef struct _MlConfig MlConfig;


/* Enumeration for using three-valued logic
 * See: https://en.wikipedia.org/wiki/Three-valued_logic */
typedef enum {
	ML_OFF,		// force disabled         (3VL: no)
	ML_ON,		// force enabled          (3VL: yes)
	ML_AUTO		// better/default choice  (3VL: unset)
} MlTristate;


typedef enum {
	ML_VALUE_TYPE_INT32,	// address is a pointer to int32_t
	ML_VALUE_TYPE_INT64,	// address is a pointer to int64_t
	ML_VALUE_TYPE_TRISTATE,	// address is a pointer to MlTristate
	ML_VALUE_TYPE_BOOLEAN,	// address is a pointer to bool
	ML_VALUE_TYPE_STRING,	// address is a pointer to char* (must be dynamically allocated, or initialized with NULL)
	ML_VALUE_TYPE_STRV		// address is a pointer to char** (must be dynamically allocated, or initialized with NULL)
} MlValueType;


MlConfig *
ml_config_new ()
ML_FN_RETURNS_NONNULL;

void
ml_config_destroy (MlConfig *config);

size_t
ml_config_sizeof (MlConfig *config)
ML_FN_SIZEOF;

void
ml_config_add_entry (MlConfig *config, char *key, MlValueType type, mlPointer address, const char *label, const char *description);

void
ml_config_add_entry_with_bounds (MlConfig *config, char *key, MlValueType type, mlPointer address, const char *label, const char *description, int64_t min, int64_t max);

MlValueType
ml_config_get_entry_type (MlConfig *config, const char *key);

const char *
ml_config_get_label (MlConfig *config, const char *key);

const char *
ml_config_get_description (MlConfig *config, const char *key);

bool
ml_config_get_entry_bounds (MlConfig *config, const char *key, int64_t *min, int64_t *max);

void
ml_config_set_entry_bounds (MlConfig *config, const char *key, int64_t min, int64_t max);

bool
ml_config_set (MlConfig *config, const char *key, const char *value)
ML_FN_WARN_UNUSED_RESULT;

const char *
ml_config_get (MlConfig *config, const char *key)
ML_FN_READ_DYNAMIC_PROPERTY;

cJSON *
ml_config_to_json (MlConfig *config)
ML_FN_COLD;

void
ml_config_parse_json (MlConfig *config, cJSON *obj);

const char * const *
ml_config_list_keys (MlConfig *config)
ML_FN_READ_DYNAMIC_PROPERTY;


ML_HEADER_END
#endif /* ML_HEADER__GRAPH_CONFIG_H__INCLUDED */
