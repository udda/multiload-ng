/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of multiload-ng.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


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
G_GNUC_INTERNAL void
multiload_ui_start_system_monitor(MultiloadPlugin *ma);
G_GNUC_INTERNAL MultiloadOptions *
multiload_ui_parse_cmdline(int *argc, char ***argv, GOptionEntry *extra_entries);

extern const char* MULTILOAD_CONFIG_PATH;

G_END_DECLS

#endif /* __UI_H__ */
