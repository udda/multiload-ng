#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include "multiload.h"

G_BEGIN_DECLS

#define PROP_CPULOAD			0
#define PROP_MEMLOAD			1
#define PROP_NETLOAD			2
#define PROP_SWAPLOAD			3
#define PROP_LOADAVG			4
#define PROP_DISKLOAD			5

#define PROP_SPEED				6
#define PROP_SIZE				7
#define PROP_PADDING			8
#define PROP_SPACING			9
#define PROP_SHOWFRAME			10

#define ACTION_DEFAULT_COLORS	1

typedef enum {
	LOADAVG_1 = 0,
	LOADAVG_5,
	LOADAVG_15
} LoadAvgType;


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
typedef struct	_LoadGraphProperties	LoadGraphProperties;


struct _MultiLoadProperties {
	LoadGraphProperties cpuload;
	LoadGraphProperties memload;
	LoadGraphProperties swapload;
	LoadGraphProperties netload;
	LoadGraphProperties loadavg;
};
typedef struct	_MultiLoadProperties	MultiLoadProperties;


G_GNUC_INTERNAL void
multiload_init_preferences(GtkWidget *dialog, MultiloadPlugin *ma);

G_END_DECLS

#endif
