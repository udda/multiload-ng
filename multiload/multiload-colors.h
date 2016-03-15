#ifndef H_MULTILOAD_COLORS_
#define H_MULTILOAD_COLORS_

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include "multiload.h"


G_BEGIN_DECLS

void multiload_colors_stringify(MultiloadPlugin *ma, guint i, char *list);
void multiload_colors_default(MultiloadPlugin *ma, guint i);
gboolean multiload_colors_unstringify(MultiloadPlugin *ma, guint i, const char *list);

gboolean multiload_colors_from_file(const gchar *filename, MultiloadPlugin *ma, GtkWindow *parent);
gboolean multiload_colors_to_file(const gchar *filename, MultiloadPlugin *ma, GtkWindow *parent);

G_END_DECLS

#endif /* H_MULTILOAD_COLORS_ */
