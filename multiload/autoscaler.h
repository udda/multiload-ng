#ifndef __AUTOSCALER_H__
#define __AUTOSCALER_H__

#include <glib.h>
#include <time.h>


G_BEGIN_DECLS

typedef struct _AutoScaler
{
	/* const */ unsigned update_interval;
	/* const */ unsigned floor;
	unsigned max;
	unsigned count;
	time_t last_update;
	float sum;
	float last_average;
} AutoScaler;


G_GNUC_INTERNAL void
autoscaler_init(AutoScaler *that, unsigned interval, unsigned floor);
G_GNUC_INTERNAL unsigned
autoscaler_get_max(AutoScaler *that, unsigned current);

G_END_DECLS

#endif /* __AUTOSCALER_H__ */
