#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PLUGIN_WEBSITE "https://github.com/nandhp/multiload-nandhp"

#define NCPUSTATES 5
#define NGRAPHS 6
#define MAX_COLORS 5

enum {
	GRAPH_CPULOAD = 0,
	GRAPH_MEMLOAD = 1,
	GRAPH_NETLOAD = 2,
	GRAPH_SWAPLOAD = 3,
	GRAPH_LOADAVG = 4,
	GRAPH_DISKLOAD = 5
};

typedef struct _MultiloadPlugin MultiloadPlugin;
typedef struct _LoadGraph LoadGraph;
typedef void (*LoadGraphDataFunc) (int, int [], LoadGraph *);
typedef struct _GraphConfig GraphConfig;
typedef struct _GraphType GraphType;

#include "netspeed.h"

#define MIN_SIZE 10
#define DEFAULT_SIZE 40
#define MAX_SIZE 400
#define STEP_SIZE 5

#define MIN_SPEED 50
#define DEFAULT_SPEED 500
#define MAX_SPEED 10000
#define STEP_SPEED 50

#define MIN_PADDING 0
#define DEFAULT_PADDING 2
#define MAX_PADDING 40
#define STEP_PADDING 1

#define MIN_SPACING 0
#define DEFAULT_SPACING 1
#define MAX_SPACING 40
#define STEP_SPACING 1

#define DEFAULT_SHOWFRAME TRUE

struct _LoadGraph {
	MultiloadPlugin *multiload;

	guint id;
	guint draw_width, draw_height;

	guint allocated;

	gint **data;
	guint data_size;
	guint *pos;

	GtkWidget *main_widget;
	GtkWidget *frame, *box, *disp;
	cairo_surface_t *surface;
	int timer_index;

	long cpu_time [NCPUSTATES];
	long cpu_last [NCPUSTATES];
	int cpu_initialized;

	double loadavg1;
	NetSpeed *netspeed_in;
	NetSpeed *netspeed_out;

	guint diskread;
	guint diskwrite;

	gboolean tooltip_update;
};

struct _GraphConfig {
	gboolean visible;
	GdkColor colors[MAX_COLORS];
};

struct _MultiloadPlugin
{
	/* Current state */
	GtkWidget *box;
	GtkOrientation orientation;
	LoadGraph *graphs[NGRAPHS];

	/* Settings */
	GtkContainer *container;
	GraphConfig graph_config[NGRAPHS];
	guint speed;
	guint size;
	guint padding;
	guint spacing;

	gboolean show_frame;
};

struct _GraphType {
	const char *interactive_label;
	const char *noninteractive_label;
	const char *name;
	LoadGraphDataFunc get_data;
	guint num_colors;
	const struct {
		const char *prefs_label;
		const char *default_value;
	} colors[MAX_COLORS];
	//const char *default_colors;
};
GraphType graph_types[NGRAPHS];

#include "load-graph.h"
#include "linux-proc.h"

/* remove the old graphs and rebuild them */
void
multiload_refresh(MultiloadPlugin *ma, GtkOrientation orientation);

/* update the tooltip to the graph's current "used" percentage */
void
multiload_tooltip_update(LoadGraph *g);

void
multiload_init();

void
multiload_destroy(MultiloadPlugin *ma);

/* Utility functions for preferences and data storage */
gboolean
multiload_gdk_color_stringify(GdkColor* color, gchar *color_string);

void
multiload_colorconfig_stringify(MultiloadPlugin *ma, guint i, char *list);

void
multiload_colorconfig_default(MultiloadPlugin *ma, guint i);

void
multiload_colorconfig_unstringify(MultiloadPlugin *ma, guint i,
								const char *list);

int
multiload_find_graph_by_name(const char *str, const char **suffix);

G_END_DECLS

#endif
