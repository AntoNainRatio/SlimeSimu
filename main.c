#include <gtk/gtk.h>
#include <cairo.h>
#include "ship.h"

#define SHIP_PERIOD 4

typedef enum State
{
	PLAY,
	PAUSE,
} State;

typedef struct UserInterface
{
	GtkWindow* window;
	GtkDrawingArea* area;
} UserInterface;

typedef struct Simu
{
	State state;
	guint event;
	UserInterface ui;
	struct ship* ship;

} Simu;

gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	Simu* simu = user_data;

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);

	GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    //int width = allocation.width;
    //int height = allocation.height;

	/*double rayon = 3;
	double x = 200;
	double y = 200;

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_arc(cr, x, y, rayon, 0, 2 * G_PI);
	cairo_fill(cr);
	*/

	drawShip(simu->ship, allocation, cr);

	return FALSE;
}


gboolean on_move_ship(gpointer user_data)
{
	Simu* simu = user_data;

	GtkAllocation allocation;
	gtk_widget_get_allocation(simu->ui.window, &allocation);

	updateShip(simu->ship);
	g_print("Ship updated\n");
	return TRUE;
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	Simu* simu = user_data;

	if( event->keyval == GDK_KEY_space)
	{
		g_print("Space pressed\n");
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
		return TRUE;
	}
	return FALSE;
}

int main (int argc, char *argv[])
{
	gtk_init (&argc, &argv);

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
	gtk_window_set_default_size(window, 900, 900);
	GtkDrawingArea* area = GTK_DRAWING_AREA(gtk_builder_get_object(builder,"drawing_area"));

	struct ship* a = getNewShip(200, 200, 0);

	Simu simu =
	{
		.state = PAUSE,
		.ui =
		{
			.window = window,
			.area = area,
		},
		.ship = a,
	};

	g_signal_connect(area, "draw", G_CALLBACK(on_draw), &simu);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(window, "key_press_event", G_CALLBACK(on_key_press), &simu);
	gtk_widget_show_all(GTK_WIDGET(window));

	g_object_unref(builder);

	gtk_main ();

	return 0;
}
