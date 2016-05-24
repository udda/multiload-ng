#ifndef __UI_H__
#define __UI_H__

#include "multiload.h"


G_BEGIN_DECLS

G_GNUC_INTERNAL void
multiload_ui_read (MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_ui_save (MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_ui_show_help();
G_GNUC_INTERNAL void
multiload_ui_show_about (GtkWindow* parent);
G_GNUC_INTERNAL GtkWidget *
multiload_ui_configure_dialog_new (MultiloadPlugin *ma, GtkWindow *parent);

G_END_DECLS

#endif /* __UI_H__ */
