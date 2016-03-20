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
#include "multiload-colors.h"
#include "linux-proc.h"
#include "util.h"

/* update the tooltip to the graph's current "used" percentage */
void
multiload_tooltip_update(LoadGraph *g)
{
	gchar *title = NULL;
	gchar *text;
	gchar *tooltip_markup;

	g_assert_nonnull(g);
	g_assert ( g->id >= 0 && g->id < GRAPH_MAX );

	switch (g->id) {
		case GRAPH_CPULOAD: {
			CpuData *xd = (CpuData*) g->extra_data;
			g_assert_nonnull(xd);


			if (g->multiload->tooltip_details) {
				gchar *uptime = format_time_duration(xd->uptime);
				title = g_strdup_printf(xd->cpu0_name);
				text = g_strdup_printf(_(	"%ld processors  -  %.2f GHz  -  Governor: %s\n"
											"%.1f%% in use by programs\n"
											"%.1f%% in wait for I/O\n"
											"%.1f%% total CPU use\n"
											"\n"
											"Uptime: %s"),
											xd->num_cpu, xd->cpu0_mhz/1000.0, xd->cpu0_governor,
											(xd->user*100),
											(xd->iowait*100),
											(xd->total_use*100),
											uptime);
				g_free(uptime);
			} else {
				text = g_strdup_printf("%.1f%%", xd->total_use*100);
			}
		}	break;

		case GRAPH_MEMLOAD: {
			MemoryData *xd = (MemoryData*) g->extra_data;
			g_assert_nonnull(xd);


			if (g->multiload->tooltip_details) {
				gchar *total = g_format_size_full(xd->total, G_FORMAT_SIZE_IEC_UNITS);
				gchar *user = format_percent(xd->user, xd->total, 1);
				gchar *cache = format_percent(xd->cache, xd->total, 1);
				title = g_strdup_printf(_("%s of RAM"), total);
				text = g_strdup_printf(_(	"%s in use by programs\n"
											"%s in use as cache"),
											user, cache);
				g_free(total);
				g_free(user);
				g_free(cache);
			} else {
				gchar *use = format_percent(xd->user+xd->cache, xd->total, 0);
				text = g_strdup_printf("%s", use);
				g_free(use);
			}
		}	break;

		case GRAPH_NETLOAD: {
			NetData *xd = (NetData*) g->extra_data;
			g_assert_nonnull(xd);

			gchar *tx_in = format_rate_for_display(xd->in_speed);
			gchar *tx_out = format_rate_for_display(xd->out_speed);
			gchar *tx_local = format_rate_for_display(xd->local_speed);

			if (g->multiload->tooltip_details) {
				text = g_strdup_printf(_(	"Monitored interfaces: %s\n"
											"\n"
											"Receiving: %s\n"
											"Sending: %s\n"
											"Local: %s"),
											xd->ifaces, tx_in, tx_out, tx_local);
			} else {
				text = g_strdup_printf("\xe2\xac\x86%s \xe2\xac\x87%s", tx_in, tx_out);
			}

			g_free(tx_in);
			g_free(tx_out);
			g_free(tx_local);
		}	break;

		case GRAPH_SWAPLOAD: {
			SwapData *xd = (SwapData*) g->extra_data;
			g_assert_nonnull(xd);

			if (xd->total == 0) {
				text = g_strdup_printf(_("No swap"));
			} else {
				gchar *used = format_percent(xd->used, xd->total, 0);
				gchar *total = g_format_size_full(xd->total, G_FORMAT_SIZE_IEC_UNITS);

				if (g->multiload->tooltip_details) {
					title = g_strdup_printf(_("%s of swap"), total);
					text = g_strdup_printf(_("%s used"), used);
				} else {
					text = g_strdup_printf("%s", used);
				}

				g_free(used);
				g_free(total);
			}
		}	break;

		case GRAPH_LOADAVG: {
			LoadAvgData *xd = (LoadAvgData*) g->extra_data;
			g_assert_nonnull(xd);

			if (g->multiload->tooltip_details) {
				text = g_strdup_printf(_(	"Last minute: %0.02f\n"
											"Last 5 minutes: %0.02f\n"
											"Last 15 minutes: %0.02f"),
											xd->loadavg[0], xd->loadavg[1], xd->loadavg[2]);
			} else {
				text = g_strdup_printf("%0.02f", xd->loadavg[0]);
			}
		}	break;

		case GRAPH_DISKLOAD: {
			DiskData *xd = (DiskData*) g->extra_data;
			g_assert_nonnull(xd);

			gchar *disk_read = format_rate_for_display(xd->read_speed);
			gchar *disk_write = format_rate_for_display(xd->write_speed);

			if (g->multiload->tooltip_details) {
				text = g_strdup_printf(_(	"Read: %s\n"
											"Write: %s"),
											disk_read, disk_write);
			} else {
				text = g_strdup_printf("\xe2\xac\x86%s \xe2\xac\x87%s", disk_read, disk_write);
			}
			g_free(disk_read);
			g_free(disk_write);
		}	break;

		case GRAPH_TEMPERATURE: {
			TemperatureData *xd = (TemperatureData*) g->extra_data;
			g_assert_nonnull(xd);

			if (g->multiload->tooltip_details) {
				text = g_strdup_printf(_(	"Current: %.1f °C\n"
											"Critical: %.1f °C"),
											(xd->value/1000.0), (xd->max/1000.0));
			} else {
				text = g_strdup_printf("%.1f °C", xd->value/1000.0);
			}
		}	break;

		default: {
			g_assert_not_reached();
		}	break;
	}

	if (title == NULL)
		title = g_strdup(graph_types[g->id].label_noninteractive);

	if (g->multiload->tooltip_details) {
		tooltip_markup = g_strdup_printf("<span weight='bold' size='larger'>%s</span>\n%s", title, text);
	} else {
		tooltip_markup = g_strdup_printf("%s: %s", title, text);
	}

	gtk_widget_set_tooltip_markup(g->disp, tooltip_markup);
	g_free(title);
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
	initialized = 1;

	glibtop *glt = glibtop_init();
	g_assert_nonnull(glt);

	multiload_config_init();
}

void multiload_defaults(MultiloadPlugin *ma)
{
	guint i;

	/* default settings */
	ma->speed = DEFAULT_SPEED;
	ma->size = DEFAULT_SIZE;
	ma->padding = DEFAULT_PADDING;
	ma->spacing = DEFAULT_SPACING;
	ma->fill_between = DEFAULT_FILL_BETWEEN;
	ma->tooltip_details = DEFAULT_TOOLTIP_DETAILS;
	ma->dblclick_policy = DEFAULT_DBLCLICK_POLICY;
	for ( i = 0; i < GRAPH_MAX; i++ ) {
		ma->graph_config[i].border_width = DEFAULT_BORDER_WIDTH;
		ma->graph_config[i].visible = i == 0 ? TRUE : FALSE;
		multiload_colors_default(ma, i);
	}
}

void
multiload_sanitize(MultiloadPlugin *ma)
{
	guint i, visible_count = 0;

	/* Keep values between max and min */
	ma->speed = CLAMP(ma->speed, MIN_SPEED, MAX_SPEED);
	ma->size = CLAMP(ma->size, MIN_SIZE, MAX_SIZE);
	ma->padding = CLAMP(ma->padding, MIN_PADDING, MAX_PADDING);
	ma->spacing = CLAMP(ma->spacing, MIN_SPACING, MAX_SPACING);
	ma->fill_between = ma->fill_between? TRUE:FALSE;
	ma->tooltip_details = ma->tooltip_details? TRUE:FALSE;
	ma->orientation_policy = CLAMP(ma->orientation_policy, 0, 2);
	ma->dblclick_policy = CLAMP(ma->dblclick_policy, 0, 2);

	for ( i=0; i<GRAPH_MAX; i++ ) {
		ma->graph_config[i].border_width =
								CLAMP(ma->graph_config[i].border_width,
								MIN_BORDER_WIDTH, MAX_BORDER_WIDTH);

		if (ma->graph_config[i].visible) {
			ma->graph_config[i].visible = TRUE;
			visible_count++;
		}
	}

	/* Ensure at lease one graph is visible */
	if (visible_count == 0)
		ma->graph_config[0].visible = TRUE;
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

