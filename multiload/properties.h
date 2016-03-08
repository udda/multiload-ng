#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include "multiload.h"

G_BEGIN_DECLS

#define PROP_CPULOAD 0
#define PROP_MEMLOAD 1
#define PROP_NETLOAD 2
#define PROP_SWAPLOAD 3
#define PROP_LOADAVG 4
#define PROP_DISKLOAD 5

typedef enum {
	LOADAVG_1 = 0,
	LOADAVG_5,
	LOADAVG_15
} LoadAvgType;

typedef struct	_MultiLoadProperties		MultiLoadProperties;

typedef struct	_LoadGraphProperties		LoadGraphProperties;

struct _LoadGraphProperties {
	guint type, n;
	const gchar *name;
	const gchar **texts;
	const gchar **color_defs;
	GdkColor *colors;
	gulong adj_data [3];
	gint loadavg_type;
	gint use_default;
};

struct _MultiLoadProperties {
	LoadGraphProperties cpuload, memload, swapload, netload, loadavg;
};

/*
void multiload_properties_apply (void) G_GNUC_INTERNAL;
void multiload_properties_close (void) G_GNUC_INTERNAL;
void multiload_properties_changed (void) G_GNUC_INTERNAL;
void multiload_show_properties (PropertyClass prop_class) G_GNUC_INTERNAL;
void multiload_init_properties (void) G_GNUC_INTERNAL;
*/
G_GNUC_INTERNAL void
multiload_init_preferences(GtkWidget *dialog, MultiloadPlugin *ma);

G_END_DECLS

#endif
