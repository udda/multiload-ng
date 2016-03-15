#ifndef __MULTILOAD_COLORS_H__
#define __MULTILOAD_COLORS_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include "multiload.h"


G_BEGIN_DECLS

G_GNUC_INTERNAL void
multiload_colors_stringify(MultiloadPlugin *ma, guint i, char *list);
G_GNUC_INTERNAL void
multiload_colors_default(MultiloadPlugin *ma, guint i);
G_GNUC_INTERNAL gboolean
multiload_colors_unstringify(MultiloadPlugin *ma, guint i, const char *list);

G_GNUC_INTERNAL gboolean
multiload_colors_from_file(const gchar *filename, MultiloadPlugin *ma, GtkWindow *parent);
G_GNUC_INTERNAL gboolean
multiload_colors_to_file(const gchar *filename, MultiloadPlugin *ma, GtkWindow *parent);

G_END_DECLS

#endif /* __MULTILOAD_COLORS_H__ */
