#include <config.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"


gchar*
format_rate_for_display(guint rate)
{
	gchar *bytes = g_format_size_full(rate, G_FORMAT_SIZE_IEC_UNITS);
	// xgettext: this is a rate format (eg. 2 MiB/s)
	gchar *ret = g_strdup_printf(_("%s/s"), bytes);

	g_free(bytes);
	return ret;
}

gchar*
format_percent(guint value, guint total, guint ndigits)
{
	gchar *ret;
	gchar *format;

	double percent = 100.0 * (double)value / (double)total;
	if (percent < 0)
		percent = 0;
	if (percent > 100)
		percent = 100;

	if (ndigits == 0) {
		ret = g_strdup_printf("%u%%", (guint)percent);
	} else {
		format = g_strdup_printf("%%.%uf%%%%", ndigits);
		ret = g_strdup_printf(format, percent);
		g_free(format);
	}

	return ret;
}


GtkWidget* gtk_spin_button_new_with_parameters(gint min, gint max, gint step, gint start_value)
{
	guint maxdigits;
	guint mindigits;
	guint maxlen;
	gchar *str;
	GtkWidget *w = gtk_spin_button_new_with_range(min, max, step);

	// calculate max length
	str = g_strdup_printf("%u", max);
	maxdigits = strlen(str);
	g_free(str);
	str = g_strdup_printf("%u", min);
	mindigits = strlen(str);
	g_free(str);

	maxlen = MAX(maxdigits, mindigits);
	gtk_entry_set_max_length (GTK_ENTRY(w), maxlen);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), (gdouble)start_value);

	return w;
}


static void
close_dialog_cb(GtkWidget *dialog, gint response, gpointer id) {
	gtk_widget_destroy(dialog);
}

void gtk_error_dialog(GtkWindow *parent, const gchar *message)
{
	GtkWidget *dialog = gtk_message_dialog_new(parent,
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, message);
	g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK(close_dialog_cb), NULL);
	gtk_widget_show(dialog);
}

GtkWidget* gtk_yesno_dialog(GtkWindow *parent, const gchar *message, GCallback cb, gpointer data)
{
	GtkWidget *dialog = gtk_message_dialog_new(parent,
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message);
	g_signal_connect (G_OBJECT (dialog), "response", cb, data);
	return dialog;
}

gchar* gtk_open_file_dialog(GtkWindow *parent, const gchar *title)
{
	int response;
	char *filename;
	GtkWidget *dialog = gtk_file_chooser_dialog_new (title, parent,
										GTK_FILE_CHOOSER_ACTION_OPEN,
										GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
										NULL);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	else
		filename = NULL;
	gtk_widget_destroy (dialog);
	return filename;
}

gchar* gtk_save_file_dialog(GtkWindow *parent, const gchar *title, const gchar *current_name)
{
	int response;
	char *filename;
	GtkWidget *dialog = gtk_file_chooser_dialog_new (title, parent,
										GTK_FILE_CHOOSER_ACTION_SAVE,
										GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
										NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_file_chooser_set_create_folders (GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), current_name);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	else
		filename = NULL;
	gtk_widget_destroy (dialog);
	return filename;
}
