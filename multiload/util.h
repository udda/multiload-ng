#ifndef H_MULTILOAD_UTIL_
#define H_MULTILOAD_UTIL_

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>


gchar* format_rate_for_display (guint rate);
gchar* format_percent(guint value, guint total, guint ndigits);

GtkWidget* gtk_spin_button_new_with_parameters(gint min, gint max, gint step, gint start_value);

void gtk_error_dialog(GtkWindow *parent, const gchar *message);
GtkWidget* gtk_yesno_dialog(GtkWindow *parent, const gchar *message, GCallback cb, gpointer data);
gchar* gtk_open_file_dialog(GtkWindow *parent, const gchar *title);
gchar* gtk_save_file_dialog(GtkWindow *parent, const gchar *title, const gchar *current_name);

#endif /* H_MULTILOAD_UTIL_ */
