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


struct _MlGraph {
	// graph settings
	MlGraphType type;
	int border_size;
	uint32_t ceiling;
	MlGraphStyle *style;
	MlConfig *config;

	// runtime data
	int width;
	int height;
	bool last_error;
	bool is_dirty;
	bool is_dirty_bg;
	bool is_dirty_caption;

	MlCaption *caption;
	const MlGraphTypeInterface *iface;

	int64_t n_collect_fail;
	int64_t n_collect_success;

	MlGraphFailFunc fail_fn;
	mlPointer fail_fn_data;

	MlSurface *bg_surface; // contains background, updated only when style changes
	MlSurface *surface;
	MlDataset *dataset;
	MlGraphContext *context;
};


MlGraph*
ml_graph_new (MlGraphType type, int width, int height, int border_size, MlGraphStyle *style)
{
	if_unlikely (style == NULL) {
		ml_error ("No style provided");
		return NULL;
	}

	if_unlikely (border_size < 0) {
		ml_error ("Passed negative value (%d) for border size", border_size);
		return NULL;
	}

	if_unlikely (width <= 0 || height <= 0) {
		ml_error ("Invalid value for width (%d) and/or height (%d) for border size", width, height);
		return NULL;
	}

	if (width <= 2*border_size) {
		ml_error ("Border (%d) too large for specified width (%d)", border_size, width);
		return NULL;
	}

	if (height <= 2*border_size) {
		ml_error ("Border (%d) too large for specified height (%d)", border_size, height);
		return NULL;
	}

	const MlGraphTypeInterface *iface = ml_graph_type_interface_get (type);
	if_unlikely (iface == NULL) {
		ml_error ("Cannot get type interface");
		return NULL;
	}

	MlGraph *g = ml_new (MlGraph);

	g->type = type;
	g->width = width;
	g->height = height;
	g->border_size = border_size;
	g->iface = iface;

	if (!ml_graph_set_style (g, style)) {
		ml_error ("Cannot create graph due to invalid style");
		ml_graph_destroy (g);
		return NULL;
	}

	g->caption = ml_caption_new ();
	g->surface = ml_surface_new (g->width, g->height);
	g->bg_surface = ml_surface_new (g->width, g->height);
	g->dataset = ml_dataset_new (g->iface->n_data, g->width - 2*g->border_size, g->iface->dataset_mode);

	g->config = ml_config_new ();
	g->context = ml_graph_context_new (g->iface, g->config);
	if_unlikely (g->context == NULL) {
		ml_graph_destroy (g);
		return NULL;
	}

	g->is_dirty = true;
	g->is_dirty_bg = true;
	g->is_dirty_caption = true;

	return g;
}

MlGraph *
ml_graph_new_from_json (cJSON *obj, int width, int height, const char *theme_name)
{
	if_unlikely (obj == NULL)
		return NULL;

	// type
	MlGraphType type = ml_graph_type_parse_json (ml_cJSON_GetObjectItem (obj, "type"));
	if (type == ML_INVALID) {
		ml_warning ("Unknown graph type \"%s\"", obj->valuestring);
		return NULL;
	}

	// border
	cJSON *obj_border = ml_cJSON_GetObjectItem (obj, "border");
	if (obj_border == NULL)
		return NULL;
	int border_size = obj_border->valueint;

	// style (optional)
	MlGraphStyle *style;
	cJSON *obj_style = cJSON_GetObjectItem (obj, "style");
	if (obj_style != NULL)
		style = ml_graph_style_new_from_json (obj_style);
	else
		style = ml_graph_theme_generate_style (theme_name, ml_graph_type_interface_get (type));

	if (style == NULL) {
		ml_warning ("Cannot generate graph style");
		return NULL;
	}

	MlGraph *g = ml_graph_new (type, width, height, border_size, style);
	if (g == NULL)
		return NULL;

	// ceiling (optional)
	cJSON *obj_ceiling = cJSON_GetObjectItem (obj, "ceiling");
	if (obj_ceiling != NULL && ml_graph_set_ceiling (g, (uint32_t)obj_ceiling->valueint))
		ml_warning ("Cannot set graph ceiling to %d", obj_ceiling->valueint);

	// config (optional)
	cJSON *obj_config = cJSON_GetObjectItem (obj, "config");
	if (obj_config != NULL) {
		ml_config_parse_json (g->config, obj_config);
		if (!ml_graph_set_config (g, NULL, NULL)) {
			ml_graph_destroy (g);
			return NULL;
		}
	}

	return g;
}


void
ml_graph_destroy (MlGraph *g)
{
	if_unlikely (g == NULL)
		return;

	ml_caption_destroy (g->caption);
	ml_surface_destroy (g->surface);
	ml_surface_destroy (g->bg_surface);
	ml_dataset_destroy (g->dataset);
	ml_graph_style_destroy (g->style);
	ml_graph_context_destroy (g->context);
	ml_config_destroy (g->config);

	free (g);
}

size_t
ml_graph_sizeof (MlGraph *g)
{
	if_unlikely (g == NULL)
		return 0;

	size_t size = sizeof (MlGraph);

	size += ml_graph_style_sizeof (g->style);
	size += ml_config_sizeof (g->config);
	size += ml_caption_sizeof (g->caption);
	size += ml_surface_sizeof (g->surface);
	size += ml_surface_sizeof (g->bg_surface);
	size += ml_dataset_sizeof (g->dataset);
	size += ml_graph_context_sizeof (g->context);

	return size;
}

bool
ml_graph_resize (MlGraph *g, int width, int height)
{
	if_unlikely (g == NULL || g->surface == NULL || g->bg_surface == NULL)
		return false;

	if (g->width == width && g->height == height)
		return true;

	if_unlikely (width < 1 || height < 1) {
		ml_error ("Invalid value for width (%d) and/or height (%d) for border size", width, height);
		return NULL;
	}

	if (width <= 2*g->border_size) {
		ml_error ("Cannot resize graph: border (%d) too large for specified width (%d)", g->border_size, width);
		return false;
	}

	if (height <= 2*g->border_size) {
		ml_error ("Cannot resize graph: border (%d) too large for specified height (%d)", g->border_size, height);
		return false;
	}

	/* NOTE: functions ml_surface_resize and ml_dataset_resize cannot actually fail,
	 * since their prerequisites are already checked by ml_graph_resize */
	if_unlikely (!ml_surface_resize (g->surface, width, height))
		return false;
	if_unlikely (!ml_surface_resize (g->bg_surface, width, height))
		return false;

	// resize dataset only if width changed
	if_unlikely (width != g->width && !ml_dataset_resize (g->dataset, width - 2*g->border_size))
		return false;

	g->width = width;
	g->height = height;

	// invalidate surfaces
	g->is_dirty = true;
	g->is_dirty_bg = true;

	return true;
}

void
ml_graph_set_fail_function (MlGraph *g, MlGraphFailFunc func, mlPointer user_data)
{
	if_unlikely (g == NULL)
		return;

	g->fail_fn = func;
	g->fail_fn_data = user_data;
}

void
ml_graph_unpause (MlGraph *g)
{
	if_unlikely (g == NULL || g->iface == NULL)
		return;

	if (g->iface->unpause_fn != NULL)
		g->iface->unpause_fn (g->context);
}

int64_t
ml_graph_get_collect_success_count (MlGraph *g)
{
	if_unlikely (g == NULL)
		return -1;

	return g->n_collect_success;
}

int64_t
ml_graph_get_collect_fail_count (MlGraph *g)
{
	if_unlikely (g == NULL)
		return -1;

	return g->n_collect_fail;
}

bool
ml_graph_get_last_collect_succeeded (MlGraph *g)
{
	if_unlikely (g == NULL)
		return false;

	return !g->last_error;
}

const char *
ml_graph_get_error_message (MlGraph *g)
{
	if_unlikely (g == NULL || g->context == NULL)
		return NULL;

	if (!g->last_error)
		return NULL;

	return ml_graph_context_get_fail_message (g->context);
}

const char *
ml_graph_get_name (MlGraph *g) {
	if_unlikely (g == NULL || g->iface == NULL)
		return NULL;

	return g->iface->name;
}

const char *
ml_graph_get_label (MlGraph *g) {
	if_unlikely (g == NULL || g->iface == NULL)
		return NULL;

	return _(g->iface->label);
}

const char *
ml_graph_get_description (MlGraph *g) {
	if_unlikely (g == NULL || g->iface == NULL)
		return NULL;

	return _(g->iface->description);
}

MlCaption *
ml_graph_get_caption (MlGraph *g)
{
	if_unlikely (g == NULL)
		return NULL;

	return g->caption;
}

void
ml_graph_update_caption (MlGraph *g)
{
	if_unlikely (g == NULL || g->caption == NULL || g->iface == NULL)
		return;

	if (g->is_dirty_caption) {
		ml_caption_clear (g->caption);

		if (!g->last_error) {
			if (g->iface->caption_fn != NULL)
				g->iface->caption_fn (g->caption, g->dataset, ml_graph_context_get_provider_data (g->context));
		} else {
			ml_caption_set (g->caption, ML_CAPTION_COMPONENT_HEADER, _("Error"));
			ml_caption_set (g->caption, ML_CAPTION_COMPONENT_BODY, ml_graph_get_error_message(g));
		}

		if (ml_string_has_suffix(ml_caption_get (g->caption, ML_CAPTION_COMPONENT_BODY), "\n"))
			ml_warning ("Body string ends with a newline character. This is probably a mistake.");

		g->is_dirty_caption = false;
	}
}


MlSurface*
ml_graph_get_surface (MlGraph *g)
{
	if_unlikely (g == NULL)
		return NULL;

	return g->surface;
}

MlConfig *
ml_graph_get_ml_config (MlGraph *g)
{
	if_unlikely (g == NULL)
		return NULL;

	return g->config;
}

const MlGraphStyle *
ml_graph_get_style (MlGraph *g)
{
	if_unlikely (g == NULL)
		return NULL;

	return (const MlGraphStyle*)g->style;
}

bool
ml_graph_set_style (MlGraph *g, MlGraphStyle *style)
{
	if_unlikely (g == NULL || g->iface == NULL || style == NULL)
		return false;

	if (style->n_data_colors < g->iface->n_data) {
		ml_error ("Cannot set style to graph '%s': not enough colors in provided style (%d required, %d provided)", g->iface->name, g->iface->n_data, style->n_data_colors);
		return false;
	}

	if (g->style != NULL)
		ml_graph_style_destroy (g->style);

	g->style = style;
	g->is_dirty_bg = true;

	return true;
}

uint32_t
ml_graph_get_ceiling (MlGraph *g)
{
	if_unlikely (g == NULL)
		return 0;

	return g->ceiling;
}

bool
ml_graph_set_ceiling (MlGraph *g, uint32_t ceiling)
{
	if_unlikely (g == NULL)
		return false;

	// ceiling = 0 : auto

	g->ceiling = ceiling;
	return true;
}

int
ml_graph_get_border_size (MlGraph *g)
{
	if_unlikely (g == NULL)
		return 0;

	return g->border_size;
}

bool
ml_graph_set_border_size (MlGraph *g, int border_size)
{
	if_unlikely (g == NULL)
		return false;

	if_unlikely (border_size < 0) {
		ml_error ("Specified negative border size (%d)", border_size);
		return false;
	}

	if (g->width <= 2*border_size) {
		ml_error ("Cannot resize graph: border (%d) too large for specified width (%d)", border_size, g->width);
		return false;
	}

	if (g->height <= 2*border_size) {
		ml_error ("Cannot resize graph: border (%d) too large for specified height (%d)", border_size, g->height);
		return false;
	}

	if_unlikely (!ml_dataset_resize (g->dataset, g->width - 2*border_size))
		return false;

	g->border_size = border_size;

	// invalidate surfaces
	g->is_dirty = true;
	g->is_dirty_bg = true;

	return true;
}

const char*
ml_graph_get_config (MlGraph *g, const char *key)
{
	if_unlikely (g == NULL || g->config == NULL || key == NULL)
		return NULL;

	return ml_config_get (g->config, key);
}

bool
ml_graph_set_config (MlGraph *g, const char *key, const char *value)
{
	if_unlikely (g == NULL || g->config == NULL)
		return false;

	if (key != NULL) { // with key = NULL this function just calls config_fn (use it to trigger config update)
		if (!ml_config_set (g->config, key, value))
			return false;
	}

	if (g->iface->config_fn != NULL) {
		ml_graph_context_before (g->context);
		g->iface->config_fn (g->context);
		ml_graph_context_after (g->context);

		if (ml_graph_context_get_need_data_reset (g->context))
			ml_dataset_clear (g->dataset);
	}

	return true;
}

bool
ml_graph_collect_data (MlGraph *g)
{
	if_unlikely (g == NULL || g->iface == NULL || g->iface->get_fn == NULL)
		return false;

	ml_graph_context_before (g->context);
	g->iface->get_fn (g->context);
	ml_graph_context_after (g->context);

	g->is_dirty = true;
	g->is_dirty_caption = true;

	int context_fail_code = ml_graph_context_get_fail_code (g->context);
	g->last_error = (context_fail_code != 0);
	if (g->last_error) {
		const char *context_fail_msg = ml_graph_context_get_fail_message (g->context);

		g->n_collect_fail++;
		if (g->fail_fn != NULL)
			g->fail_fn (g, context_fail_code, context_fail_msg, g->fail_fn_data);

		/* data collect failed, but technically there is "new data" (the
		   error itself), so we return true to make MlMultiloadTimer work */
		return true;
	}

	g->n_collect_success++;
	ml_graph_context_push_to_dataset (g->context, g->dataset);

	return true;
}

void
ml_graph_draw (MlGraph *g)
{
	if_unlikely (g == NULL || g->style == NULL)
		return;

	if_likely (!g->is_dirty)
		return;

	if_unlikely (g->is_dirty_bg) {
		ml_surface_fill_background (g->bg_surface, &g->style->background_gradient);
		ml_surface_draw_border (g->bg_surface, &g->style->border_color, g->border_size);
		g->is_dirty_bg = false;
	}

	ml_surface_copy_surface (g->surface, 0, 0, g->bg_surface);

	ml_surface_draw_dataset (g->surface, g->dataset, g->border_size, g->style->data_colors, g->ceiling);
	if (g->last_error)
		ml_surface_draw_stripes (g->surface, &g->style->error_color);

	g->is_dirty = false;
}

cJSON *
ml_graph_to_json (MlGraph *g)
{
	if_unlikely (g == NULL)
		return NULL;

	cJSON* obj = cJSON_CreateObject ();

	cJSON_AddItemToObject	(obj, "type",		ml_graph_type_to_json(g->type));
	cJSON_AddNumberToObject (obj, "border",		g->border_size);
	cJSON_AddNumberToObject (obj, "ceiling",	g->ceiling);

	cJSON_AddItemToObject	(obj, "config",		ml_config_to_json (g->config));
	cJSON_AddItemToObject	(obj, "style",		ml_graph_style_to_json (g->style));

	return obj;
}

void
ml_graph_dump_dataset (MlGraph *g)
{
	if_unlikely (g != NULL)
		ml_dataset_dump (g->dataset);
}
