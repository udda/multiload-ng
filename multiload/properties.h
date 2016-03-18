#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include "multiload.h"


G_BEGIN_DECLS

// properties and actions are all in the first 16 bits; this make room for another 16 bit value
enum {
	PROP_SHOWGRAPH			= 0x00010000,
	PROP_SPEED				= 0x00020000,
	PROP_SIZE				= 0x00030000,
	PROP_PADDING			= 0x00040000,
	PROP_SPACING			= 0x00050000,
	PROP_ORIENTATION		= 0x00060000,
	PROP_BORDERWIDTH		= 0x00070000,
	PROP_COLOR				= 0x00080000,
	PROP_FILLBETWEEN		= 0x00090000,
	PROP_TOOLTIP_DETAILS	= 0x000A0000
};


enum {
	ACTION_DEFAULT_COLORS	= 0x01000000,
	ACTION_EXPORT_COLORS	= 0x02000000,
	ACTION_IMPORT_COLORS	= 0x03000000
};


G_GNUC_INTERNAL void
multiload_init_preferences(GtkWidget *dialog, MultiloadPlugin *ma);

G_END_DECLS

#endif /* __PROPERTIES_H__ */
