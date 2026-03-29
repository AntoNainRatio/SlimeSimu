#include <gtk/gtk.h>
#include <cairo.h>
#include "phero.h"
#include "ship.h"
#include "simu.h"

#define SHIP_PERIOD 3
#define SHIPNUMBER 300

#define WIDTH 1800
#define HEIGHT 900

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	//cairo_set_source_rgb(surface_cr, 0, 0, 0);
	//cairo_paint(surface_cr);

	//cairo_t *surface_cr = cairo_create(simu->ui.surface);

	redraw(user_data, cr);


	//cairo_set_source_surface(cr, simu->ui.surface, 0, 0);
	//cairo_paint(cr);

	return FALSE;
}

gboolean on_move_ship(gpointer user_data)
{
	Simu* simu = user_data;

	updateSimu(simu);

	/*cairo_t *surface_cr = cairo_create(simu->ui.surface);
	redraw(user_data, surface_cr);
	cairo_destroy(surface_cr);*/

	gtk_widget_queue_draw(GTK_WIDGET(simu->ui.area));

	return TRUE;
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	Simu* simu = user_data;

	if( event->keyval == GDK_KEY_space)
	{
		if ((simu->state) == PAUSE)
		{
			simu->state = PLAY;
			simu->event = g_timeout_add(SHIP_PERIOD, on_move_ship, simu);
		}
		else
		{
			simu->state = PAUSE;
			g_source_remove(simu->event);
			simu->event = 0;
		}
		//return TRUE;
	}
	//return FALSE;
	return TRUE;
}

int main (int argc, char *argv[])
{
	gtk_init (&argc, &argv);

	srand(time(NULL));

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if(gtk_builder_add_from_file(builder, "template.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		g_object_unref(builder);
		return 1;
	}

	GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder,"window"));

	int width = WIDTH;
	int height = HEIGHT;

	gtk_window_set_default_size(window, width, height);
	GtkDrawingArea* area = GTK_DRAWING_AREA(gtk_builder_get_object(builder,"drawing_area"));

	float shipNumber = SHIPNUMBER;
	if (argc == 2){
		shipNumber = atoi(argv[1]);
	}

	Simu simu = getNewSimu(window, area, width, height, shipNumber);

	g_signal_connect(area, "draw", G_CALLBACK(on_draw), &simu);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(window, "key_press_event", G_CALLBACK(on_key_press), &simu);
	gtk_widget_show_all(GTK_WIDGET(window));

	g_object_unref(builder);

	gtk_main();

	freeSimu(simu);
	return 0;
}
