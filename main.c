#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include "jdecode.h"

#define draw_pixel(cr, x, y) \
	do { \
	cairo_rectangle((cr), (x), (y), 1, 1); \
	cairo_fill((cr)); \
	} while (0)

#define draw_line(cr, x0, y0, x1, y1) \
	do { \
	cairo_move_to((cr), (x0), (y0)); \
	cairo_line_to((cr), (x1), (y1)); \
	cairo_stroke((cr)); \
	} while (0)

char filename[20];

static gboolean on_expose_event(GtkWidget *widget,
		GdkEventExpose *event, gpointer data);


int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *drawable;

	memset(filename, 0, 20);
	memcpy(filename, argv[1], strlen(argv[1]));

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Draw Line");
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
	g_signal_connect(GTK_OBJECT(window), "destroy",
			G_CALLBACK(gtk_main_quit), NULL);

	drawable = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window), drawable);
	g_signal_connect(GTK_OBJECT(drawable), "expose-event",
			G_CALLBACK(on_expose_event), (gpointer)window);

	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}

static gboolean on_expose_event(GtkWidget *widget,
		GdkEventExpose *event, gpointer data)
{
	cairo_t *cr;
	int width, height, i, j;
	struct block_8x8 *p;
	int x0, y0;

	gtk_window_get_size(GTK_WINDOW(data), &width, &height);
	cr = gdk_cairo_create(widget->window);

	cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);

	jd_init(filename);

	p = jd_first_8x8_block();
	while (p != NULL) {
		x0 = p->x0;
		y0 = p->y0;
		for (i = 0; i < 8; i++) {
			for (j = 0; j < 8; j++) {
				cairo_set_source_rgb(cr,
					p->r[i * 8 + j] / 256.0,
					p->g[i * 8 + j] / 256.0,
					p->b[i * 8 + j] / 256.0);
				draw_pixel(cr, x0 + j, y0 + i);
			}
		}

		p = jd_next_8x8_block();
	}

	jd_exit();

	return FALSE;
}
