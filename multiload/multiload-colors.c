#include <config.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "multiload.h"
#include "multiload-colors.h"
#include "multiload-config.h"
#include "util.h"


static void
gdk_color_to_argb_string(GdkColor* color, guint16 alpha, gchar *out_str)
{
	// note: out_str must be at least 10 characters long
	int rc = snprintf(out_str, 10, "#%02X%02X%02X%02X",
					alpha / 256,
					color->red / 256,
					color->green / 256,
					color->blue / 256);
	g_assert(rc == 9);
}

static gboolean
argb_string_to_gdk_color(const gchar *gspec, GdkColor *color, guint16 *alpha)
{
	gchar buf[8];
	if (strlen(gspec) == 7) {
		// may be a standard RGB hex string, fallback to gdk_color_parse
		return gdk_color_parse(gspec, color);
	} else if (G_UNLIKELY (strlen(gspec) != 9) ) {
		return FALSE;
	}

	// alpha part
	buf[0] = gspec[1];
	buf[1] = gspec[2];
	buf[2] = 0;
	errno = 0;
	*alpha = (guint16)strtol(buf, NULL, 16);
	if (errno) {
		// error in strtol, set alpha=max
		*alpha = 0xFFFF;
	} else {
		/* alpha is in the form '0x00jk'. Transform it in the form
		  '0xjkjk', so the conversion of 8 to 16 bits is proportional. */
		*alpha |= (*alpha << 8);
	}

	// color part
	buf[0] = '#';
	strncpy(buf+1, gspec+3, 6);
	buf[7] = 0;
	return gdk_color_parse(buf, color);
}




/* Convert graph configuration into a string of the form "#aarrggbb,#aarrggbb,..."
   Output string must have size at least 10*MAX_COLORS. */
void
multiload_colors_stringify(MultiloadPlugin *ma, guint i, char *list)
{
	guint ncolors = multiload_config_get_num_colors(i);
	guint j;
	GdkColor *colors = ma->graph_config[i].colors;
	guint16 *alphas = ma->graph_config[i].alpha;
	char *listpos = list;

	if ( G_UNLIKELY (!list) )
		return;

	// Create color list
	for ( j = 0; j < ncolors; j++ ) {
		gdk_color_to_argb_string(&colors[j], alphas[j], listpos);
		if ( j == ncolors-1 )
			listpos[9] = 0;
		else
			listpos[9] = ',';
		listpos += 10;
	}
	g_assert (strlen(list) == 10*ncolors-1);
}


/* Set the colors for graph i to the default values */
void
multiload_colors_default(MultiloadPlugin *ma, guint i)
{
	guint j;
	for ( j = 0; j < multiload_config_get_num_colors(i); j++ ) {
		argb_string_to_gdk_color(graph_types[i].colors[j].default_value,
						&ma->graph_config[i].colors[j],
						&ma->graph_config[i].alpha[j]);
	}
}

/* Set graph colors from a string, as produced by multiload_colors_stringify */
gboolean
multiload_colors_unstringify(MultiloadPlugin *ma, guint i, const char *list)
{
	guint ncolors = multiload_config_get_num_colors(i);
	guint j;
	GdkColor *colors = ma->graph_config[i].colors;
	guint16 *alphas = ma->graph_config[i].alpha;
	const char *listpos = list;

	if ( G_UNLIKELY (!listpos) ) {
		multiload_colors_default(ma, i);
		return FALSE;
	}

	for ( j = 0; j < ncolors; j++ ) {
		/* Check the length of the list item. */
		int pos = 0;
		if ( j == ncolors-1 )
			pos = strlen(listpos);
		else
			pos = (int)(strchr(listpos, ',')-listpos);

		/* Try to parse the color */
		if ( G_UNLIKELY (pos != 9) ) {
			multiload_colors_default(ma, i);
			return FALSE;
		}

		/* Extract the color into a null-terminated buffer */
		char buf[10];
		strncpy(buf, listpos, 9);
		buf[9] = 0;
		if ( G_UNLIKELY (argb_string_to_gdk_color(buf, &colors[j], &alphas[j]) != TRUE) ) {
			multiload_colors_default(ma, i);
			return FALSE;
		}

		listpos += 10;
	}

	//ignore alpha value of last two colors (background and border)
	alphas[ncolors-1] = 0xFFFF;
	alphas[ncolors-2] = 0xFFFF;

	return TRUE;
}


gboolean multiload_colors_from_file(const gchar *filename, MultiloadPlugin *ma, GtkWindow *parent)
{
	char *line = NULL;
	char *color_str;
	size_t len = NULL;
	ssize_t read;
	int graph;
	gboolean first_line = TRUE;
	gboolean status = TRUE;
	FILE *f = fopen(filename, "r");

	if (f == NULL) {
		gtk_error_dialog(parent, _("Could not open the file."));
		return FALSE;
	}

	while ((read = getline(&line, &len, f)) != -1) {
		// remove newline
		if (line[read-1] == '\n')
			line[read-1] = 0;

		if (first_line) {
			first_line = FALSE;
			if (strcmp(line, "MULTILOAD") != 0) {
				gtk_error_dialog(parent, _("Wrong file format."));
				status = FALSE;
				break;
			}
			continue;
		}

		graph = multiload_find_graph_by_name(line, &color_str);
		// remove leading space
		color_str++;

		if (multiload_colors_unstringify(ma, graph, color_str) != TRUE) {
			gtk_error_dialog(parent, _("Wrong file format."));
			status = FALSE;
			break;
		}
		multiload_init_preferences(parent, ma);
		multiload_refresh(ma);
	}

	fclose(f);
	if (line)
		g_free(line);

	return status;
}

gboolean multiload_colors_to_file(const gchar *filename, MultiloadPlugin *ma, GtkWindow *parent)
{
	gint i;
	FILE *f = fopen(filename, "w");
	gchar color_str[10 * MAX_COLORS];
	if (f == NULL) {
		gtk_error_dialog(parent, _("Could not save the file."));
		return FALSE;
	}
	fprintf(f, "MULTILOAD\n");
	for ( i=0; i<GRAPH_MAX; i++) {
		multiload_colors_stringify (ma, i, color_str);
		fprintf(f, "%s %s\n", graph_types[i].name, color_str);
	}
	fclose(f);
	return TRUE;
}

