#ifndef H_MULTILOAD_MULTILOAD_CONFIG_
#define H_MULTILOAD_MULTILOAD_CONFIG_

#include <glib.h>
#include <glib/gi18n-lib.h>

#include "multiload.h"


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



guint multiload_config_get_num_colors(guint id);

guint multiload_config_get_num_data(guint id);

void multiload_config_init();


#endif /* H_MULTILOAD_MULTILOAD_CONFIG_ */
