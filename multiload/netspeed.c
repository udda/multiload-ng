#include <config.h>
#include <glib.h>
#include <time.h>

#include "multiload.h"
#include "netspeed.h"
#include "util.h"

enum { N_STATES = 4 };

struct _NetSpeed
{
	LoadGraph *graph;
	gulong states[N_STATES];
	size_t cur;
};

NetSpeed* netspeed_new(LoadGraph *g)
{
	NetSpeed *ns = g_new0(NetSpeed, 1);
	ns->graph = g;
	return ns;
}

void netspeed_delete(NetSpeed *ns)
{
	g_free(ns);
}

void netspeed_add(NetSpeed *ns, gulong tx)
{
	ns->cur = (ns->cur + 1) % N_STATES;
	ns->states[ns->cur] = tx;
}

char* netspeed_get(NetSpeed *ns)
{
	gulong older, newer;
	guint rate;

	newer = ns->states[ns->cur];
	older = ns->states[(ns->cur + 1) % N_STATES];

	if ((older != 0) && (newer > older))
		rate = (newer - older) * 1000 / ((N_STATES - 1) *
				  ns->graph->multiload->speed);
	else
		/*	We end up here if we haven't got enough data yet or the
			network interface has jumped back (or there has never
			been any activity on any interface). A value of 0 is
			likely to be accurate, but if it is wrong it will be
			clearly wrong. In any event, it should fix itself in a
			few seconds. */
		rate = 0;

	return format_rate_for_display(rate);
}
