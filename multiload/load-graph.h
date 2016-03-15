#ifndef __LOAD_GRAPH_H__
#define __LOAD_GRAPH_H__

#include "multiload.h"


G_BEGIN_DECLS

G_GNUC_INTERNAL LoadGraph*
load_graph_new (MultiloadPlugin *ma, guint id);
G_GNUC_INTERNAL void
load_graph_resize (LoadGraph *g);
G_GNUC_INTERNAL void
load_graph_start (LoadGraph *g);
G_GNUC_INTERNAL void
load_graph_stop (LoadGraph *g);
G_GNUC_INTERNAL void
load_graph_unalloc (LoadGraph *g);

G_END_DECLS

#endif /* __LOAD_GRAPH_H__ */
