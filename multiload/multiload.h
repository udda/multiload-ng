#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PLUGIN_WEBSITE "https://github.com/nandhp/multiload-nandhp"

#define PER_CPU_MAX_LOADAVG 4
#define NCPUSTATES 5

#define MAX_COLORS 6

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

typedef struct _MultiloadPlugin MultiloadPlugin;
typedef struct _LoadGraph LoadGraph;
typedef struct _GraphConfig GraphConfig;

#include "netspeed.h"

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

#define DEFAULT_SHOWFRAME TRUE
#define DEFAULT_ORIENTATION MULTILOAD_ORIENTATION_AUTO

struct _LoadGraph {
	MultiloadPlugin *multiload;

	guint id;
	guint draw_width, draw_height;

	gint **data;
	guint *pos;

	GtkWidget *main_widget;
	GtkWidget *frame, *box, *disp;
	cairo_surface_t *surface;
	int timer_index;

	// cpu load
	long cpu_time [NCPUSTATES];
	long cpu_last [NCPUSTATES];
	int cpu_initialized;

	// net load
	NetSpeed *netspeed_in;
	NetSpeed *netspeed_out;

	// load average
	double loadavg[3];

	// disk load
	guint diskread;
	guint diskwrite;

	// temperature
	guint temperature;

	gboolean allocated;
	gboolean tooltip_update;
};

struct _GraphConfig {
	gboolean visible;
	GdkColor colors[MAX_COLORS];
	guint16 alpha[MAX_COLORS];
};

struct _MultiloadPlugin
{
	/* Current state */
	GtkWidget *box;
	GtkOrientation panel_orientation;
	LoadGraph *graphs[GRAPH_MAX];

	/* Settings */
	GtkContainer *container;
	GraphConfig graph_config[GRAPH_MAX];
	guint orientation_policy;
	guint speed;
	guint size;
	guint padding;
	guint spacing;

	gboolean show_frame;
};



#include "load-graph.h"
#include "linux-proc.h"

/* remove the old graphs and rebuild them */
void
multiload_refresh(MultiloadPlugin *ma);

/* get current orientation */
GtkOrientation
multiload_get_orientation(MultiloadPlugin *ma);

/* update the tooltip to the graph's current "used" percentage */
void
multiload_tooltip_update(LoadGraph *g);

void
multiload_init();

void
multiload_destroy(MultiloadPlugin *ma);

void
multiload_colorconfig_stringify(MultiloadPlugin *ma, guint i, char *list);

void
multiload_colorconfig_default(MultiloadPlugin *ma, guint i);

void
multiload_colorconfig_unstringify(MultiloadPlugin *ma, guint i,
								const char *list);


G_END_DECLS

#endif
