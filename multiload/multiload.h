#ifndef __MULTILOAD_H__
#define __MULTILOAD_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>


#define PLUGIN_WEBSITE "https://github.com/nandhp/multiload-nandhp"

#define MAX_COLORS 7

enum {
	GRAPH_CPULOAD,
	GRAPH_MEMLOAD,
	GRAPH_NETLOAD,
	GRAPH_SWAPLOAD,
	GRAPH_LOADAVG,
	GRAPH_DISKLOAD,
	GRAPH_TEMPERATURE,

	GRAPH_MAX
};

#define MULTILOAD_ORIENTATION_AUTO			0
#define MULTILOAD_ORIENTATION_HORIZONTAL	1
#define MULTILOAD_ORIENTATION_VERTICAL		2

#define DBLCLICK_POLICY_DONOTHING		0
#define DBLCLICK_POLICY_TASKMANAGER		1
#define DBLCLICK_POLICY_CMDLINE			2

#define MIN_SIZE 10
#define DEFAULT_SIZE 40
#define MAX_SIZE 400
#define STEP_SIZE 5

#define MIN_SPEED 50
#define DEFAULT_SPEED 500
#define MAX_SPEED 20000
#define STEP_SPEED 50

#define MIN_PADDING 0
#define DEFAULT_PADDING 2
#define MAX_PADDING 40
#define STEP_PADDING 1

#define MIN_SPACING 0
#define DEFAULT_SPACING 1
#define MAX_SPACING 40
#define STEP_SPACING 1

#define MIN_BORDER_WIDTH 0
#define DEFAULT_BORDER_WIDTH 1
#define MAX_BORDER_WIDTH 16
#define STEP_BORDER_WIDTH 1

#define DEFAULT_ORIENTATION MULTILOAD_ORIENTATION_AUTO
#define DEFAULT_DBLCLICK_POLICY DBLCLICK_POLICY_DONOTHING
#define DEFAULT_FILL_BETWEEN FALSE
#define DEFAULT_TOOLTIP_DETAILS FALSE

typedef struct _LoadGraph LoadGraph;

typedef struct _GraphConfig {
	gboolean visible;
	gint border_width;
	GdkColor colors[MAX_COLORS];
	guint16 alpha[MAX_COLORS];
} GraphConfig;

typedef struct _MultiloadPlugin {
	/* Current state */
	GtkWidget *box;
	GtkOrientation panel_orientation;
	LoadGraph *graphs[GRAPH_MAX];

	/* Settings */
	GtkContainer *container;
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


//#include "load-graph.h"
//#include "linux-proc.h"


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
