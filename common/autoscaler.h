#ifndef __AUTOSCALER_H__
#define __AUTOSCALER_H__

#include <glib.h>
#include <time.h>

#include "multiload.h"

G_BEGIN_DECLS

/* Min value of the the maximum. Note that the autoscaler
 * is used to represent transfer rates (byte/sec) */
#define AUTOSCALER_FLOOR 500

typedef struct {
	unsigned max;
	unsigned count;
	time_t last_update;
	float sum;
	float last_average;
} AutoScaler;


G_GNUC_INTERNAL unsigned
autoscaler_get_max(AutoScaler *s, LoadGraph *g, unsigned current);

G_END_DECLS

#endif /* __AUTOSCALER_H__ */
