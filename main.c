#include <gtk/gtk.h>

int main (int argc, char *argv[])
{
    gtk_init (&argc, &argv);
   	
	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if(gtk_builder_add_from_file(builder, "template.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		printf("zizi\n");
		return 1;
	}

    GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder,"window"));
    GtkDrawingArea* area = GTK_DRAWING_AREA(gtk_builder_get_object(builder,"drawing_area"));
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	gtk_main ();
    
    return 0;
}
