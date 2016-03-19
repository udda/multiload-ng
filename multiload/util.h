#ifndef __UTIL_H__
#define __UTIL_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

G_GNUC_INTERNAL gint64
calculate_speed(guint64 delta, guint period_ms);

G_GNUC_INTERNAL gboolean
file_check_contents(FILE *f, const gchar *string);
G_GNUC_INTERNAL gint64
read_int_from_file(const gchar *path);

G_GNUC_INTERNAL gchar*
format_rate_for_display (guint rate);
G_GNUC_INTERNAL gchar*
format_percent(guint value, guint total, guint ndigits);
G_GNUC_INTERNAL gchar*
format_time_duration(gdouble seconds);

G_GNUC_INTERNAL GtkWidget*
gtk_spin_button_new_with_parameters(gint min, gint max, gint step, gint start_value, const gchar *format);
G_GNUC_INTERNAL GtkWidget*
gtk_warning_bar_new(const gchar *text);

G_GNUC_INTERNAL void
gtk_error_dialog(GtkWindow *parent, const gchar *message);
G_GNUC_INTERNAL GtkWidget*
gtk_yesno_dialog(GtkWindow *parent, const gchar *message, GCallback cb, gpointer data);
G_GNUC_INTERNAL gchar*
gtk_open_file_dialog(GtkWindow *parent, const gchar *title);
G_GNUC_INTERNAL gchar*
gtk_save_file_dialog(GtkWindow *parent, const gchar *title, const gchar *current_name);

G_GNUC_INTERNAL const gchar*
get_system_monitor_executable();

G_END_DECLS

#endif /* __UTIL_H__ */
