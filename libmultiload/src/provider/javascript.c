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


const MlGraphTypeInterface ML_PROVIDER_JAVASCRIPT_IFACE = {
	.name			= "javascript",
	.label			= N_("JavaScript interpreter"),
	.description	= N_("Customizable output using JavaScript language."),

	.hue			= 17,

	.n_data			= 4,
	.dataset_mode	= ML_DATASET_MODE_ABSOLUTE,

	.init_fn		= ml_provider_javascript_init,
	.config_fn		= ml_provider_javascript_config,
	.get_fn			= ml_provider_javascript_get,
	.destroy_fn		= ml_provider_javascript_destroy,
	.sizeof_fn		= ml_provider_javascript_sizeof,
	.unpause_fn		= NULL,
	.caption_fn		= NULL,

	.helptext		= N_(
		"The following special functions are available:\n"
		"<ul>"
		"<li> <b>multiloadPutData(...)</b><br />Push data to graph. Every argument is a NON NEGATIVE number to push (4 max arguments). Multiple calls overwrite previous values. </li>"
		"<li> <b>multiloadPutTotal(n)</b><br />Set total value. If not set, it will be calculated from provided data. </li>"
		"<li> <b>multiloadPrintMessage(message)</b><br />Prints a informative message, usually to console. </li>"
		"<li> <b>multiloadReadFile(filename)</b><br />Reads a local file and returns its contents. </li>"
		"<li> <b>multiloadExecuteCommand(cmdline, read_exit_code)</b><br />Executes a command line. If <i>read_exit_code</i> is true, returns exit code, if false, returns stdout contents. </li>"
		"<li> <b>multiloadGetEnv(key)</b><br />Returns value of environment variable, or <i>undefined</i> if key is not found. </li>"
		"</ul>"
		// TODO MULTILOAD_MIN_DATA_VALUE, MULTILOAD_MAX_DATA_VALUE
	)
};


typedef struct {
	char *js_eval_full;
	MlGrowBuffer *gbuf_stdout;
	MlGrowBuffer *gbuf_report;

	char *js_code;
	int32_t exec_timeout;
} JAVASCRIPTstate;


#define ML_CONTEXT_SYMBOL_NAME "__Multiload_JS_Context__"

static MlGraphContext *
_js_get_context (duk_context *ctx)
{
	if (duk_get_global_string (ctx, ML_CONTEXT_SYMBOL_NAME) == 0)
		return NULL;

	MlGraphContext *context = (MlGraphContext*)duk_get_pointer (ctx, -1);
	return context;
}

static duk_ret_t
_js_put_data (duk_context *ctx)
{
	int n_args = duk_get_top(ctx);

	MlGraphContext *context = _js_get_context (ctx);
	if (context == NULL)
		return 0;

	for (int i = 0; i < n_args; i++) {
		uint32_t v = duk_get_uint (ctx, i); // returns 0 for invalid numbers (http://duktape.org/api.html#duk_get_uint), it's fine for us
		ml_graph_context_set_data (context, i, v);
	}

	return 0;
}

static duk_ret_t
_js_put_total (duk_context *ctx)
{
	MlGraphContext *context = _js_get_context (ctx);
	if (context == NULL)
		return 0;

	uint32_t v = duk_get_uint (ctx, 0); // returns 0 for invalid numbers (http://duktape.org/api.html#duk_get_uint), it's fine for us
	ml_graph_context_set_max (context, v);

	return 0;
}

static duk_ret_t
_js_print (duk_context *ctx)
{
	const char *str = duk_get_string (ctx, 0);
	if_likely (str != NULL)
		ml_info (_("Message from JavaScript: %s"), str);

	return 0;
}

static duk_ret_t
_js_read_file (duk_context *ctx)
{
	char *str = NULL;
	size_t len;

	const char *filename = duk_get_string (ctx, 0);
	if_unlikely (filename == NULL)
		duk_push_undefined (ctx);
	else if (ml_infofile_read_string (filename, &str, &len))
		duk_push_lstring (ctx, str, len);
	else
		duk_push_null (ctx);

	if_likely (str != NULL)
		free (str);

	return 1;
}

static duk_ret_t
_js_execute_command (duk_context *ctx)
{
	MlGraphContext *context = _js_get_context (ctx);
	if (context == NULL) {
		duk_push_null (ctx);
		return 1;
	}

	JAVASCRIPTstate *s = (JAVASCRIPTstate*)ml_graph_context_get_provider_data (context);
	if_unlikely (s == NULL) {
		ml_warning ("Cannot retrieve context");
		duk_push_null (ctx);
		return 1;
	}

	int exit_status;

	bool read_exit_code = duk_to_boolean (ctx, 0);

	const char *cmdline = duk_get_string (ctx, 1);
	if_unlikely (cmdline == NULL) {
		duk_push_undefined (ctx);
		return 1;
	}

	if (ml_execute_cmdline_sync (cmdline, NULL, s->exec_timeout, s->gbuf_stdout, NULL, s->gbuf_report, &exit_status)) {
		if (read_exit_code)
			duk_push_int (ctx, exit_status);
		else
			duk_push_string (ctx, ml_grow_buffer_get_string (s->gbuf_stdout));
	} else {
		ml_warning (ml_grow_buffer_get_string (s->gbuf_report));
		duk_push_null (ctx);
	}

	return 1;
}

static duk_ret_t
_js_getenv (duk_context *ctx) {
	const char *name = duk_get_string (ctx, 0);

	if (name == NULL)
		duk_push_undefined (ctx);
	else {
		const char *value = getenv (name);
		if (value == NULL)
			duk_push_undefined (ctx);
		else
			duk_push_string (ctx, value);
	}

	return 1;
}

static void
_duk_fatal_handler (ML_UNUSED void *user_data, const char *msg) {
	ml_error (_("Duktape fatal error: %s"), msg);
}


mlPointer
ml_provider_javascript_init (MlConfig *config)
{
	JAVASCRIPTstate *s = ml_new (JAVASCRIPTstate);
	s->js_eval_full = NULL;
	s->gbuf_stdout = ml_grow_buffer_new (500);
	s->gbuf_report = ml_grow_buffer_new (1);

	s->js_code = NULL;
	s->exec_timeout = 500;

	ml_config_add_entry (config,
		"js_code",
		ML_VALUE_TYPE_STRING,
		&s->js_code,
		_("Javascript code"),
		_("Code to be executed at every graph iteration, to provide values to draw.")
	);

	ml_config_add_entry (config,
		"exec_timeout",
		ML_VALUE_TYPE_INT32,
		&s->exec_timeout,
		_("Execution timeout"),
		_("Number of milliseconds to wait before terminating child process started with multiloadExecuteCommand. Set to -1 to wait indefinitely.")
	);

	return s;
}

void
ml_provider_javascript_config (MlGraphContext *context)
{
	JAVASCRIPTstate *s = (JAVASCRIPTstate*)ml_graph_context_get_provider_data (context);
	if_unlikely (s == NULL)
		return;

	free (s->js_eval_full);

	if (s->js_code != NULL)
		s->js_eval_full = ml_strdup_printf("(function() {\n%s\n})();", s->js_code);
}

void
ml_provider_javascript_get (MlGraphContext *context)
{
	JAVASCRIPTstate *s = (JAVASCRIPTstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	ml_graph_context_assert_with_message (context, s->js_code != NULL && s->js_code[0] != '\0', _("No Javascript code"));

	duk_context *duk_ctx = duk_create_heap (NULL, NULL, NULL, context, _duk_fatal_handler);
	ml_graph_context_assert_with_message (context, duk_ctx != NULL, _("Cannot create Javascript context"));


	// multiloadPutData(...) function: every argument is a number to push (max 4). Multiple calls overwrite previous values
	duk_push_c_function (duk_ctx, _js_put_data, DUK_VARARGS);
	duk_put_global_string (duk_ctx, "multiloadPutData");

	// multiloadPutTotal(...) function: set dataset max
	duk_push_c_function (duk_ctx, _js_put_total, 1);
	duk_put_global_string (duk_ctx, "multiloadPutTotal");

	// multiloadPrintMessage(...) function: prints a message to console
	duk_push_c_function (duk_ctx, _js_print, 1);
	duk_put_global_string (duk_ctx, "multiloadPrintMessage");

	// multiloadReadFile(filename) function
	duk_push_c_function (duk_ctx, _js_read_file, 1);
	duk_put_global_string (duk_ctx, "multiloadReadFile");

	// multiloadExecuteCommand(cmdline, read_exit_code) function: if read_exit_code is true, returns exit code, else returns stdout contents
	duk_push_c_function (duk_ctx, _js_execute_command, 2);
	duk_put_global_string (duk_ctx, "multiloadExecuteCommand");

	// multiloadGetEnv(name) function: returns value of environment variable, or undefined if key is not found
	duk_push_c_function (duk_ctx, _js_getenv, 1);
	duk_put_global_string (duk_ctx, "multiloadGetEnv");

	// pointer to context
	duk_push_pointer (duk_ctx, context);
	duk_put_global_string (duk_ctx, ML_CONTEXT_SYMBOL_NAME);

	// min and max value of number data
	duk_push_uint (duk_ctx, 0);
	duk_put_global_string (duk_ctx, "MULTILOAD_MIN_DATA_VALUE");

	duk_push_uint (duk_ctx, UINT32_MAX);
	duk_put_global_string (duk_ctx, "MULTILOAD_MAX_DATA_VALUE");


	int ret = duk_peval_string (duk_ctx, s->js_eval_full);
	const char *msg = NULL;
	if (ret != 0)
		msg = duk_safe_to_string (duk_ctx, -1);

	duk_destroy_heap (duk_ctx);

	ml_graph_context_assert_with_message (context, ret == 0, _("Javascript error: %s"), msg);
}

void
ml_provider_javascript_destroy (mlPointer provider_data)
{
	JAVASCRIPTstate *s = (JAVASCRIPTstate*)provider_data;

	if_likely (s != NULL) {
		free (s->js_eval_full);
		free (s->js_code);
		ml_grow_buffer_destroy (s->gbuf_stdout, true);
		ml_grow_buffer_destroy (s->gbuf_report, true);
		free (s);
	}
}

size_t
ml_provider_javascript_sizeof (mlPointer provider_data)
{
	JAVASCRIPTstate *s = (JAVASCRIPTstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	size_t size = sizeof (JAVASCRIPTstate);

	size += ml_string_sizeof (s->js_eval_full);
	size += ml_string_sizeof (s->js_code);
	size += ml_grow_buffer_sizeof (s->gbuf_stdout);
	size += ml_grow_buffer_sizeof (s->gbuf_report);

	return size;
}
