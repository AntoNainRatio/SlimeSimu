#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <stdlib.h>
#include <string.h>
#include "phero.h"
#include "ship.h"
#include "simu.h"

#define SHIP_PERIOD      6
#define SHIPNUMBER_DEFAULT 15000

// Appelé quand le contexte GL est prêt
static void on_realize(GtkGLArea* area, gpointer user_data)
{
    gtk_gl_area_make_current(area);
    if(gtk_gl_area_get_error(area) != NULL)
    {
        g_printerr("Erreur GL lors du realize\n");
        return;
    }
    initGL((Simu*)user_data);
}

// Appelé à chaque frame
static gboolean on_draw(GtkGLArea* area, GdkGLContext* context,
                        gpointer user_data)
{
    redraw(user_data);
    return TRUE;
}

gboolean on_move_ship(gpointer user_data)
{
    Simu* simu = user_data;
    updateSimu(simu);
    gtk_widget_queue_draw(GTK_WIDGET(simu->ui.area));
    return TRUE;
}

static void on_unrealize(GtkGLArea* area, gpointer user_data)
{
    Simu* simu = user_data;
    if (simu->state == PLAY)
    {
        g_source_remove(simu->event);
        simu->event = 0;
        simu->state = PAUSE;
    }
    gtk_gl_area_make_current(area);
    freeSimu(*simu);
}

gboolean on_button_press(GtkWidget* widget, GdkEventButton* event,
                         gpointer user_data)
{
    Simu* simu = user_data;
    if      (event->button == 1) simu->mouse_mode = 1;
    else if (event->button == 3) simu->stunned    = !simu->stunned;
    simu->mouse_x = (int)event->x;
    simu->mouse_y = (int)event->y;
    return TRUE;
}

gboolean on_button_release(GtkWidget* widget, GdkEventButton* event,
                            gpointer user_data)
{
    if (event->button == 1)
        ((Simu*)user_data)->mouse_mode = 0;
    return TRUE;
}

gboolean on_motion_notify(GtkWidget* widget, GdkEventMotion* event,
                           gpointer user_data)
{
    Simu* simu = user_data;
    simu->mouse_x = (int)event->x;
    simu->mouse_y = (int)event->y;
    return TRUE;
}

gboolean on_key_press(GtkWidget* widget, GdkEventKey* event,
                      gpointer user_data)
{
    Simu* simu = user_data;
    if(event->keyval == GDK_KEY_space)
    {
        if(simu->state == PAUSE)
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
    }
    return TRUE;
}

int main(int argc, char* argv[])
{
    int ship_number = SHIPNUMBER_DEFAULT;
    for(int i = 1; i < argc; i++)
    {
        if((strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--ships") == 0) && i + 1 < argc)
        {
            ship_number = atoi(argv[++i]);
            if(ship_number <= 0)
            {
                g_printerr("Nombre de fourmis invalide: %s\n", argv[i]);
                return 1;
            }
        }
    }

    gtk_init(&argc, &argv);
    srand(time(NULL));

    GtkBuilder* builder = gtk_builder_new();
    GError*     error   = NULL;
    if(gtk_builder_add_from_file(builder, "template.glade", &error) == 0)
    {
        g_printerr("Error loading file: %s\n", error->message);
        g_clear_error(&error);
        g_object_unref(builder);
        return 1;
    }

    GtkWindow*  window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
    GtkGLArea*  area   = GTK_GL_AREA(gtk_builder_get_object(builder, "drawing_area"));

    int width  = 1800;
    int height = 900;
    gtk_window_set_default_size(window, width, height);

    Simu simu = getNewSimu(window, area, width, height, ship_number);

    g_signal_connect(area,   "realize",       G_CALLBACK(on_realize),   &simu);
    g_signal_connect(area,   "render",        G_CALLBACK(on_draw),      &simu);
    g_signal_connect(area,   "unrealize",     G_CALLBACK(on_unrealize), &simu);
    g_signal_connect(window, "destroy",       G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "key_press_event", G_CALLBACK(on_key_press), &simu);

    gtk_widget_add_events(GTK_WIDGET(area),
        GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    g_signal_connect(area, "button-press-event",   G_CALLBACK(on_button_press),   &simu);
    g_signal_connect(area, "button-release-event", G_CALLBACK(on_button_release), &simu);
    g_signal_connect(area, "motion-notify-event",  G_CALLBACK(on_motion_notify),  &simu);

    gtk_widget_show_all(GTK_WIDGET(window));
    g_object_unref(builder);
    gtk_main();
    return 0;
}
