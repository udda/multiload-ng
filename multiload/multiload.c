/* GNOME multiload panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Tim P. Gerla
 *          Martin Baulig
 *          Todd Kulesza
 *
 * With code from wmload.c, v0.9.2, apparently by Ryan Land, rland@bc1.com.
 *
 */

#include <config.h>
#include <glibtop.h>
#include "multiload.h"

/* update the tooltip to the graph's current "used" percentage */
void
multiload_tooltip_update(LoadGraph *g)
{
	gchar *tooltip_text;
	const gchar *name;

	g_assert(g);

	/* label the tooltip intuitively */
	if ( g->id >= 0 && g->id < NGRAPHS )
		name = graph_types[g->id].noninteractive_label;
	else
		g_assert_not_reached();

	switch (g->id) {
		case GRAPH_MEMLOAD:
			{
			guint mem_user, mem_cache, user_percent, cache_percent;
			mem_user  = g->data[0][0];
			mem_cache = g->data[0][1] + g->data[0][2] + g->data[0][3];
			user_percent = 100.0f * mem_user / g->draw_height;
			cache_percent = 100.0f * mem_cache / g->draw_height;
			user_percent = MIN(user_percent, 100);
			cache_percent = MIN(cache_percent, 100);

			/* xgettext: use and cache are > 1 most of the time,
			please assume that they always are.
			*/
			tooltip_text = g_strdup_printf(_("%s:\n"
							"%u%% in use by programs\n"
							"%u%% in use as cache"),
							name,
							user_percent,
							cache_percent);
			}
			break;
		case GRAPH_NETLOAD:
			{
			char *tx_in, *tx_out;
			tx_in = netspeed_get(g->netspeed_in);
			tx_out = netspeed_get(g->netspeed_out);
			/* xgettext: same as in graphic tab of g-s-m */
			tooltip_text = g_strdup_printf(_("%s:\n"
							"Receiving %s\n"
							"Sending %s"),
							name, tx_in, tx_out);
							g_free(tx_in);
							g_free(tx_out);
			}
			break;
		case GRAPH_LOADAVG:
			tooltip_text = g_strdup_printf(_("The system load average is %0.02f"),
							g->loadavg1);
			break;
		default:
			{
			const char *msg;
			guint i, total_used, percent;
			guint num_colors = graph_types[g->id].num_colors;

			for (i = 0, total_used = 0; i < (num_colors - 1); i++)
				total_used += g->data[0][i];

			percent = 100.0f * total_used / g->draw_height;
			percent = MIN(percent, 100);

			msg = ngettext("%s:\n"
							"%u%% in use",
							"%s:\n"
							"%u%% in use",
							percent);

			tooltip_text = g_strdup_printf(msg, name, percent);
			}
			break;
	}

	gtk_widget_set_tooltip_text(g->disp, tooltip_text);		
	g_free(tooltip_text);
}

static void
multiload_create_graphs(MultiloadPlugin *ma)
{
	gint i;

	for (i = 0; i < G_N_ELEMENTS (graph_types); i++) {
		g_assert (graph_types[i].num_colors <= MAX_COLORS);
		ma->graphs[i] = load_graph_new (ma, i);
	}
}

/* remove the old graphs and rebuild them */
void
multiload_refresh(MultiloadPlugin *ma, GtkOrientation orientation)
{
	gint i;

	/* stop and free the old graphs */
	for (i = 0; i < NGRAPHS; i++) {
		if (!ma->graphs[i])
			continue;
	
		load_graph_stop(ma->graphs[i]);
		gtk_widget_destroy(ma->graphs[i]->main_widget);

		load_graph_unalloc(ma->graphs[i]);
		g_free(ma->graphs[i]);
	}

	if (ma->box)
		gtk_widget_destroy(ma->box);

	/* Can we do gtk_orientable_set_orientation? */
	if ( orientation == GTK_ORIENTATION_HORIZONTAL )
		ma->box = gtk_hbox_new (FALSE, 0);
	else
		ma->box = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (ma->box);
	gtk_container_add (ma->container, ma->box);
	ma->orientation = orientation;
	/* We use show/hide to control graph visibility. Don't let a rogue panel
	 * screw that up. */
	gtk_widget_set_no_show_all (ma->box, TRUE);

	/* Create the NGRAPHS graphs, with user-configurable properties taken
	 * from ma->graph_config. */
	multiload_create_graphs (ma);

	/* only start and display the graphs the user has turned on */

	for (i = 0; i < NGRAPHS; i++) {
		gtk_box_pack_start(GTK_BOX(ma->box), 
				   ma->graphs[i]->main_widget, 
				   TRUE, TRUE, 1);
		if (ma->graph_config[i].visible) {
			gtk_widget_show_all (ma->graphs[i]->main_widget);
			load_graph_start(ma->graphs[i]);
		}
	}
		
	return;
}

void
multiload_init()
{
	static int initialized = 0;
	if ( initialized )
		return;

	glibtop *glt = glibtop_init();
	g_assert(glt != NULL);

	/* Prepare graph types */
	GraphType temp[] = {
		/* prefs_label       tooltip_label      name       get_data num_colors */
		{ _("_Processor"),   _("Processor"),    "cpuload",  GetLoad,     5,
		{ { _("_User"), "#0072b3" }, { _("_System"), "#0092e6" },
		{ _("N_ice"), "#00a3ff" }, { _("I_OWait"), "#002f3d" },
		{ _("Idl_e"), "#000000" } } },

		{ _("_Memory"),      _("Memory"),       "memload",  GetMemory,   5,
		{ { _("_User"), "#00b35b" }, { _("_Shared"), "#00e675" },
		{ _("_Buffers"), "#00ff82" }, { _("Cach_ed"), "#AAF5D0" },
		{ _("_Free"), "#000000" } } },

		{ _("_Network"),     _("Network"),      "netload",  GetNet,      4,
		{ { _("_In"), "#fce94f" }, { _("O_ut"), "#edd400" },
		{ _("L_ocal"), "#c4a000" },  { _("_Background"), "#000000" } } },

		{ _("S_wap Space"),  _("Swap Space"),   "swapload", GetSwap,     2,
		{ { _("_Used"), "#8b00c3" }, { _("_Free"), "#000000" } } },

		{ _("_Load"),        _("Load Average"), "loadavg",  GetLoadAvg,  2,
		{ { _("A_verage"), "#d50000" }, { _("_Background"), "#000000" } } },

		{ _("_Disk"),        _("Disk"),         "diskload", GetDiskLoad, 3,
		{ { _("_Read"), "#C65000" }, { _("Wr_ite"), "#FF6700" },
		{ _("_Background"), "#000000" } } }
	};
	memcpy(&graph_types, &temp, sizeof(graph_types));
}

void
multiload_destroy(MultiloadPlugin *ma)
{
	gint i;

	/* Stop the graphs */
	for (i = 0; i < NGRAPHS; i++) {
		load_graph_stop(ma->graphs[i]);
		gtk_widget_destroy(ma->graphs[i]->main_widget);

		load_graph_unalloc(ma->graphs[i]);
		g_free(ma->graphs[i]);
	}

	return;
}

/* Convert a GdkColor into a string of the form "#aabbcc" Output string must
   have size at least 8.
 */
gboolean
multiload_gdk_color_stringify(GdkColor* color, gchar *color_string)
{
  int rc = snprintf(color_string, 8, "#%02X%02X%02X", 
					color->red / 256, color->green / 256, color->blue / 256);
  gboolean retval = (rc == 7);
  g_assert(retval);
  return retval; 
}

/* Convert a graph configuration into a string of the form
   "#aabbcc,#ddeeff,...". Output string must have size at least 8*MAX_COLORS.
 */
void
multiload_colorconfig_stringify(MultiloadPlugin *ma, guint i, char *list)
{
	guint ncolors = graph_types[i].num_colors, j;
	GdkColor *colors = ma->graph_config[i].colors;
	char *listpos = list;

	if ( G_UNLIKELY (!list) )
		return;

	/* Create color list */
	for ( j = 0; j < ncolors; j++ ) {
		multiload_gdk_color_stringify(&colors[j], listpos);
		if ( j == ncolors-1 )
			listpos[7] = 0;
		else
			listpos[7] = ',';
		listpos += 8;
	}
	g_assert (strlen(list) == 8*ncolors-1);
}

/* Set the colors for graph i to the default values */
void
multiload_colorconfig_default(MultiloadPlugin *ma, guint i)
{
	guint j;
	for ( j = 0; j < graph_types[i].num_colors; j++ )
	gdk_color_parse(graph_types[i].colors[j].default_value,
					&ma->graph_config[i].colors[j]);
}

/* Set the colors for a graph from a string, as produced by
   multiload_colorconfig_stringify
 */
void
multiload_colorconfig_unstringify(MultiloadPlugin *ma, guint i,
								  const char *list)
{
	guint ncolors = graph_types[i].num_colors, j;
	GdkColor *colors = ma->graph_config[i].colors;
	const char *listpos = list;

	if ( G_UNLIKELY (!listpos) )
		return multiload_colorconfig_default(ma, i);

	for ( j = 0; j < ncolors; j++ ) {
		/* Check the length of the list item. */
		int pos = 0;
		if ( j == ncolors-1 )
			pos = strlen(listpos);
		else
			pos = (int)(strchr(listpos, ',')-listpos);

		/* Try to parse the color */
		if ( G_UNLIKELY (pos != 7) )
			return multiload_colorconfig_default(ma, i);

		/* Extract the color into a null-terminated buffer */
		char buf[8];
		strncpy(buf, listpos, 7);
		buf[7] = 0;
		if ( G_UNLIKELY (gdk_color_parse(buf, &colors[j]) != TRUE) )
			return multiload_colorconfig_default(ma, i);

		listpos += 8;
	}
}

int
multiload_find_graph_by_name(const char *str, const char **suffix)
{
	guint i;
	for ( i = 0; i < NGRAPHS; i++ ) {
		int n = strlen(graph_types[i].name);
		if ( strncasecmp(str, graph_types[i].name, n) == 0 ) {
			if ( suffix )
				*suffix = str+n;
			return i;
		}
	}
	return -1;
}
