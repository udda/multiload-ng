#include <config.h>
#include <time.h>
#include <glib.h>

#include "autoscaler.h"


unsigned autoscaler_get_max(AutoScaler *s, LoadGraph *g, unsigned current)
{
	time_t now;

	s->sum += current;
	s->count++;
	time(&now);

	if ((float)difftime(now, s->last_update) > (g->draw_width * g->multiload->interval / 1000)) {
		float new_average = s->sum / s->count;
		float average;

		if (new_average < s->last_average)
			average = ((s->last_average * 0.5f) + new_average) / 1.5f;
		else
			average = new_average;

		s->max = average * 1.2f;

		s->sum = 0.0f;
		s->count = 0;
		s->last_update = now;
		s->last_average = average;
	}

	s->max = MAX(s->max, current);
	s->max = MAX(s->max, AUTOSCALER_FLOOR);

	return s->max;
}
