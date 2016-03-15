#ifndef __NETSPEED_H__
#define __NETSPEED_H__

#include <glib.h>

#include "multiload.h"

#define N_STATES 4


G_BEGIN_DECLS

typedef struct _NetSpeed
{
	LoadGraph *graph;
	gulong states[N_STATES];
	size_t cur;
} NetSpeed;


G_GNUC_INTERNAL NetSpeed*
netspeed_new(LoadGraph *graph);
G_GNUC_INTERNAL void
netspeed_delete(NetSpeed *ns);
G_GNUC_INTERNAL void
netspeed_add(NetSpeed *ns, gulong tx);
G_GNUC_INTERNAL char*
netspeed_get(NetSpeed *ns);

G_END_DECLS

#endif /* __NETSPEED_H__ */
