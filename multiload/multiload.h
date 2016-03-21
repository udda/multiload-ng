#ifndef __MULTILOAD_H__
#define __MULTILOAD_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>


#define MAX_COLORS 7

enum GraphType {
	GRAPH_CPULOAD,
	GRAPH_MEMLOAD,
	GRAPH_NETLOAD,
	GRAPH_SWAPLOAD,
	GRAPH_LOADAVG,
	GRAPH_DISKLOAD,
	GRAPH_TEMPERATURE,

	GRAPH_MAX
};


typedef struct _LoadGraph LoadGraph;

typedef struct _GraphConfig {
	gboolean visible;
	gint border_width;
	GdkColor colors[MAX_COLORS];
	guint16 alpha[MAX_COLORS];
} GraphConfig;

typedef struct _MultiloadPlugin {
	GtkContainer *container;
	GtkWidget *box;
	GtkOrientation panel_orientation;
	LoadGraph *graphs[GRAPH_MAX];

	GraphConfig graph_config[GRAPH_MAX];
	gint speed;
	gint size;
	gint padding;
	gint spacing;
	gboolean fill_between;
	gboolean tooltip_details;
	gint orientation_policy;
	gint dblclick_policy;
	gchar dblclick_cmdline[200];
} MultiloadPlugin;


struct _LoadGraph {
	MultiloadPlugin *multiload;

	guint id;
	guint draw_width, draw_height;

	gint **data;
	guint *pos;

	GtkWidget *main_widget;
	GtkWidget *border, *box, *disp;
	cairo_surface_t *surface;
	int timer_index;

	gboolean allocated;
	gboolean tooltip_update;

	// data depend on graph type
	gpointer *extra_data;
};


G_BEGIN_DECLS

G_GNUC_INTERNAL void
multiload_refresh(MultiloadPlugin *ma);
G_GNUC_INTERNAL GtkOrientation
multiload_get_orientation(MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_tooltip_update(LoadGraph *g);
G_GNUC_INTERNAL void
multiload_init();
G_GNUC_INTERNAL void
multiload_sanitize(MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_defaults(MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_destroy(MultiloadPlugin *ma);
G_GNUC_INTERNAL int
multiload_find_graph_by_name(char *str, char **suffix);

G_END_DECLS

#endif /* __MULTILOAD_H__ */
