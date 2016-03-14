#ifndef H_MULTILOAD_NETSPEED_
#define H_MULTILOAD_NETSPEED_

#include <glib.h>

#include "multiload.h"

typedef struct _NetSpeed NetSpeed;

G_GNUC_INTERNAL NetSpeed* netspeed_new(LoadGraph *graph);
G_GNUC_INTERNAL void netspeed_delete(NetSpeed *ns);
G_GNUC_INTERNAL void netspeed_add(NetSpeed *ns, gulong tx);
G_GNUC_INTERNAL char* netspeed_get(NetSpeed *ns);

#endif /* H_MULTILOAD_NETSPEED_ */
