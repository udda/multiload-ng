#include <time.h>
#include <glib.h>

#include "autoscaler.h"

/* i wish i could have used C99 initializers instead of writing this function */
void autoscaler_init(AutoScaler *that, unsigned interval, unsigned floor)
{
	that->update_interval = interval;
	that->floor = floor;
	that->max = 0;
	that->count = 0;
	that->last_update = 0;
	that->sum =  0.0f;
	that->last_average = 0.0f;
}


unsigned autoscaler_get_max(AutoScaler *that, unsigned current)
{
	time_t now;

	that->sum += current;
	that->count++;
	time(&now);

	if ((float)difftime(now, that->last_update) > that->update_interval)
	{
		float new_average = that->sum / that->count;
		float average;

		if (new_average < that->last_average)
			average = ((that->last_average * 0.5f) + new_average) / 1.5f;
		else
			average = new_average;

		that->max = average * 1.2f;

		that->sum = 0.0f;
		that->count = 0;
		that->last_update = now;
		that->last_average = average;
	}

	that->max = MAX(that->max, current);
	that->max = MAX(that->max, that->floor);
#if 0
	printf("%p max = %u, current = %u, last_average = %f\n", that, that->max, current, that->last_average);
#endif
	return that->max;
}
