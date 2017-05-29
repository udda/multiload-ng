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

#ifndef ML_HEADER__GRAPH_TYPE_H__INCLUDED
#define ML_HEADER__GRAPH_TYPE_H__INCLUDED
ML_HEADER_BEGIN


typedef enum {
	ML_GRAPH_TYPE_BATTERY,
	ML_GRAPH_TYPE_CPU,
	ML_GRAPH_TYPE_CPUFREQ,
	ML_GRAPH_TYPE_DISKIO,
	ML_GRAPH_TYPE_ENTROPY,
	ML_GRAPH_TYPE_INTRRATE,
	ML_GRAPH_TYPE_JAVASCRIPT,
	ML_GRAPH_TYPE_LOADAVG,
	ML_GRAPH_TYPE_LOADAVG_FULL,
	ML_GRAPH_TYPE_NET,
	ML_GRAPH_TYPE_PARAMETRIC,
	ML_GRAPH_TYPE_PING,
	ML_GRAPH_TYPE_PROCRATE,
	ML_GRAPH_TYPE_RAM,
	ML_GRAPH_TYPE_RANDOM,
	ML_GRAPH_TYPE_SOCKETS,
	ML_GRAPH_TYPE_STORAGE,
	ML_GRAPH_TYPE_SWAP,
	ML_GRAPH_TYPE_TEMPERATURE,
	ML_GRAPH_TYPE_TESTFAIL,
	ML_GRAPH_TYPE_THREADS,
	ML_GRAPH_TYPE_WIFI,

	ML_GRAPH_TYPE_MAX
} MlGraphType;


typedef mlPointer	(*MlProviderInitFunc)		(MlConfig *config);
typedef void		(*MlProviderConfigFunc)		(MlGraphContext *context);
typedef void		(*MlProviderGetFunc)		(MlGraphContext *context);
typedef void		(*MlProviderUnpauseFunc)	(MlGraphContext *context);
typedef void		(*MlProviderCaptionFunc)	(MlCaption *caption, MlDataset *ds, mlPointer provider_data);


typedef struct {
	char*					name;			// internal codename - fixed, ASCII lowercase, no spaces
	char*					label;			// external name - localized, can contain spaces, single line
	char*					description;	// short description - localized, single line
	char*					helptext;		// help text - localized, multiple lines

	int						hue;			// base hue for graph style

	int						n_data;
	MlDatasetMode			dataset_mode;

	MlProviderInitFunc		init_fn;		// called when graph is initialized, to create auxiliary data
	MlProviderConfigFunc	config_fn;		// called when graph config changes
	MlProviderGetFunc		get_fn;			// called when graph needs data
	MlDestroyFunc			destroy_fn;		// called when graph is destroyed, to destroy auxiliary data
	MlSizeofFunc			sizeof_fn;		// called when ml_graph_context_sizeof needs to know the size of auxiliary data
	MlProviderUnpauseFunc	unpause_fn;		// called when graph is resumed after being stopped
	MlProviderCaptionFunc	caption_fn;		// called when graph caption needs to be filled
} MlGraphTypeInterface;


const MlGraphTypeInterface *
ml_graph_type_interface_get (MlGraphType type)
ML_FN_PURE;

const char *
ml_graph_type_to_string (MlGraphType type)
ML_FN_PURE;

MlGraphType
ml_graph_type_parse (const char *def)
ML_FN_PURE;

cJSON *
ml_graph_type_to_json (MlGraphType type);

MlGraphType
ml_graph_type_parse_json (cJSON* obj)
ML_FN_PURE ML_FN_COLD;

ML_HEADER_END
#endif /* ML_HEADER__GRAPH_TYPE_H__INCLUDED */
