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
#include "multiload-config.h"
#include "util.h"

/* update the tooltip to the graph's current "used" percentage */
void
multiload_tooltip_update(LoadGraph *g)
{
	gchar *text;
	gchar *tooltip_markup;
	const gchar *name;

	g_assert(g);

	/* label the tooltip intuitively */
	if ( g->id >= 0 && g->id < GRAPH_MAX )
		name = graph_types[g->id].label_noninteractive;
	else
		g_assert_not_reached();

	switch (g->id) {
		case GRAPH_CPULOAD: {
			guint total_used = g->data[0][0] + g->data[0][1] + g->data[0][2] + g->data[0][3];
			gchar *percent = format_percent(total_used, g->draw_height, 0);

			text = g_strdup_printf(_(	"%s in use"),
										percent);

			g_free(percent);
		}	break;

		case GRAPH_MEMLOAD: {
			guint mem_user  = g->data[0][0];
			guint mem_cache = g->data[0][1] + g->data[0][2] + g->data[0][3];
			gchar *user_percent = format_percent(mem_user, g->draw_height, 0);
			gchar *cache_percent = format_percent(mem_cache, g->draw_height, 0);

			// xgettext: use and cache are > 1 most of the time, assume that they always are.
			text = g_strdup_printf(_(	"%s in use by programs\n"
										"%s in use as cache"),
										user_percent, cache_percent);
			g_free(user_percent);
			g_free(cache_percent);
		}	break;

		case GRAPH_NETLOAD: {
			gchar *tx_in = netspeed_get(g->netspeed_in);
			gchar *tx_out = netspeed_get(g->netspeed_out);

			text = g_strdup_printf(_(	"Receiving %s\n"
										"Sending %s"),
										tx_in, tx_out);
			g_free(tx_in);
			g_free(tx_out);
		}	break;

		case GRAPH_SWAPLOAD: {
			gchar *percent = format_percent(g->data[0][0], g->draw_height, 0);

			text = g_strdup_printf(_("%s in use"), percent);

			g_free(percent);
		}	break;

		case GRAPH_LOADAVG: {
			text = g_strdup_printf(_(	"Last minute: %0.02f\n"
										"Last 5 minutes: %0.02f\n"
										"Last 15 minutes: %0.02f"),
										g->loadavg[0], g->loadavg[1], g->loadavg[2]);
		}	break;

		case GRAPH_DISKLOAD: {
			gchar *disk_read = format_rate_for_display(g->diskread);
			gchar *disk_write = format_rate_for_display(g->diskwrite);

			text = g_strdup_printf(_(	"Read %s\n"
										"Write %s"),
										disk_read, disk_write);

			g_free(disk_read);
			g_free(disk_write);
		}	break;

		case GRAPH_TEMPERATURE: {
			text = g_strdup_printf(_(	"%.1f °C"),
										(g->temperature/1000.0));
		}	break;

		default: {
			guint i;
			gchar *percent;
			guint total_used = 0;

			for (i = 0; i < multiload_config_get_num_data(g->id); i++)
				total_used += g->data[0][i];

			percent = format_percent(total_used, g->draw_height, 0);
			text = g_strdup_printf(_(	"%s in use"),
										percent);

			g_free(percent);
		}	break;
	}

	tooltip_markup = g_strdup_printf("<span underline='single' weight='bold' size='larger'>%s</span>\n%s", name, text);

	gtk_widget_set_tooltip_markup(g->disp, tooltip_markup);
	g_free(text);
	g_free(tooltip_markup);
}

/* get current orientation */
GtkOrientation
multiload_get_orientation(MultiloadPlugin *ma) {
	if (ma->orientation_policy == MULTILOAD_ORIENTATION_HORIZONTAL)
		return GTK_ORIENTATION_HORIZONTAL;
	else if (ma->orientation_policy == MULTILOAD_ORIENTATION_VERTICAL)
		return GTK_ORIENTATION_VERTICAL;
	else // if (ma->orientation_policy == MULTILOAD_ORIENTATION_AUTO)
		return ma->panel_orientation;
}

/* remove the old graphs and rebuild them */
void
multiload_refresh(MultiloadPlugin *ma)
{
	gint i;

	// stop and free the old graphs
	for (i = 0; i < GRAPH_MAX; i++) {
		if (!ma->graphs[i])
			continue;

		load_graph_stop(ma->graphs[i]);
		gtk_widget_destroy(ma->graphs[i]->main_widget);

		load_graph_unalloc(ma->graphs[i]);
		g_free(ma->graphs[i]);
	}

	if (ma->box)
		gtk_widget_destroy(ma->box);

	ma->box = gtk_vbox_new (FALSE, ma->spacing);
	gtk_container_set_border_width(GTK_CONTAINER(ma->box), ma->padding);

	// Switch between GtkVBox and GtkHBox depending of orientation
	gtk_orientable_set_orientation(GTK_ORIENTABLE(ma->box), multiload_get_orientation(ma));

	gtk_event_box_set_visible_window(GTK_EVENT_BOX(ma->container), ma->fill_between);

	gtk_widget_show (ma->box);
	gtk_container_add (ma->container, ma->box);

	// Children (graphs) are individually shown/hidden to control visibility
	gtk_widget_set_no_show_all (ma->box, TRUE);

	// Create the GRAPH_MAX graphs, with user properties from ma->graph_config.
	// Only start and display the graphs the user has turned on
	for (i = 0; i < GRAPH_MAX; i++) {
		ma->graphs[i] = load_graph_new (ma, i);

		gtk_box_pack_start(GTK_BOX(ma->box),
						   ma->graphs[i]->main_widget,
						   TRUE, TRUE, 0);

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

	multiload_config_init();
}

void
multiload_destroy(MultiloadPlugin *ma)
{
	gint i;

	/* Stop the graphs */
	for (i = 0; i < GRAPH_MAX; i++) {
		load_graph_stop(ma->graphs[i]);
		gtk_widget_destroy(ma->graphs[i]->main_widget);

		load_graph_unalloc(ma->graphs[i]);
		g_free(ma->graphs[i]);
	}

	return;
}


int
multiload_find_graph_by_name(char *str, char **suffix)
{
	guint i;
	for ( i = 0; i < GRAPH_MAX; i++ ) {
		int n = strlen(graph_types[i].name);
		if ( strncasecmp(str, graph_types[i].name, n) == 0 ) {
			if ( suffix )
				*suffix = str+n;
			return i;
		}
	}
	return -1;
}

