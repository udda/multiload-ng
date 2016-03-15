#ifndef __MULTILOAD_CONFIG_H__
#define __MULTILOAD_CONFIG_H__

#include <glib.h>
#include <glib/gi18n-lib.h>

#include "multiload.h"


G_BEGIN_DECLS

typedef void (*LoadGraphDataFunc) (int, int [], LoadGraph *);

typedef struct _GraphType {
	const char *name;
	const char *label_interactive;
	const char *label_noninteractive;
	LoadGraphDataFunc get_data;
	guint num_colors;
	const struct {
		const char *label_interactive;
		const char *label_noninteractive;
		const char *default_value;
	} colors[MAX_COLORS];
} GraphType;


// global variable
GraphType graph_types[GRAPH_MAX];


G_GNUC_INTERNAL guint
multiload_config_get_num_colors(guint id);
G_GNUC_INTERNAL guint
multiload_config_get_num_data(guint id);
G_GNUC_INTERNAL void
multiload_config_init();

G_END_DECLS


#endif /* __MULTILOAD_CONFIG_H__ */
