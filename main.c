#include <gtk/gtk.h>
#include <cairo.h>
#include "ship.h"

gboolean on_draw(GtkWidget *widget, cairo_t *cr)
{
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);

	GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    //int width = allocation.width;
    //int height = allocation.height;

	double rayon = 40;
	double x = 200;
	double y = 200;

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_arc(cr, x, y, rayon, 0, 2 * G_PI);
	cairo_fill(cr);

	return FALSE;
}

int main (int argc, char *argv[])
{
    GdkRectangle rect = { 0, 0, 10, 10};

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
    
    g_signal_connect(area, "draw", G_CALLBACK(on_draw), &rect);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	gtk_widget_show_all(GTK_WIDGET(window));
	
	g_object_unref(builder);

	gtk_main ();
    
    return 0;
}
