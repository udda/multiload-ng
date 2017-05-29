/*
 * Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of Multiload-ng.
 * 
 * Multiload-ng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Multiload-ng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <multiload.h>


const MlGraphTypeInterface ML_PROVIDER_TESTFAIL_IFACE = {
	.name			= "testfail",
	.label			= N_("Failure test"),
	.description	= N_("Fails at defined interval."),

	.hue			= 40,

	.n_data			= 1,
	.dataset_mode	= ML_DATASET_MODE_PROPORTIONAL,

	.init_fn		= ml_provider_testfail_init,
	.config_fn		= NULL,
	.get_fn			= ml_provider_testfail_get,
	.destroy_fn		= free,
	.sizeof_fn		= ml_provider_testfail_sizeof,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_testfail_caption,
	
	.helptext		= N_(
		"A simple graph that can be used for theme testing.<br />"
		"It does nothing. Its only purpose is to fail at defined interval."
	)
};


typedef struct {
	int n;
} TESTFAILstate;


#define FAIL_FREQ 4

mlPointer
ml_provider_testfail_init (ML_UNUSED MlConfig *config)
{
	TESTFAILstate *s = ml_new (TESTFAILstate);
	return s;
}

void
ml_provider_testfail_get (MlGraphContext *context)
{
	TESTFAILstate *s = (TESTFAILstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	s->n ++;

	ml_graph_context_assert_with_message (context, s->n%FAIL_FREQ == 0, _("This graph fails, that's normal"));

	ml_graph_context_set_data (context, 0, (s->n/FAIL_FREQ)%FAIL_FREQ);
	ml_graph_context_set_max  (context, FAIL_FREQ);
}

size_t
ml_provider_testfail_sizeof (mlPointer provider_data)
{
	TESTFAILstate *s = (TESTFAILstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	return sizeof (TESTFAILstate);
}

void
ml_provider_testfail_caption (MlCaption *caption, ML_UNUSED MlDataset *ds, ML_UNUSED mlPointer provider_data)
{
	ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("This graph fails every %d iterations."), FAIL_FREQ);
}
