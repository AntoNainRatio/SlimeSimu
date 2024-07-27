#include <gtk/gtk.h>
#include <cairo.h>
#include "phero.h"
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
	int width;
	int height;
} UserInterface;

typedef struct Simu
{
	State state;
	guint event;
	UserInterface ui;
	struct ship** ship;
	int shipNumber;
	float* board;

} Simu;

struct ship** getDebugShipsList()
{
	struct ship** res = malloc(1 * sizeof(struct ship*));
	//res[0] = getNewShip(450,100,200);
	res[0] = getNewShip(450,450,45);
	return res;
}

struct ship** getShipsList(int n, int width, int height)
{
	struct ship** res = malloc(n * sizeof(struct ship*));
	for(int i = 0; i < n; i++)
	{
		float angle = getNewRandomAngle(0,359);
		int x = getRandomPosition(TURNRANGE, width-TURNRANGE);
		int y = getRandomPosition(TURNRANGE, height-TURNRANGE);
		struct ship* tmp = getNewShip(x,y,angle);
		res[i] = tmp;
	}
	return res;
}

void redrawShips(gpointer user_data, cairo_t *cr)
{
	Simu* simu = user_data;
	cairo_set_source_rgb(cr, 0, 0, 0);
	//cairo_arc(cr, simu->ship->preX, simu->ship->preY, RAYON, 0, 2 * G_PI);
	cairo_paint(cr);

	cairo_set_source_rgb(cr, 1, 1, 1);
	for(int i = 0; i < simu->shipNumber; i++)
	{
		cairo_arc(cr, simu->ship[i]->x, simu->ship[i]->y, RAYON, 0, 2 * G_PI);
		cairo_fill(cr);
	}
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	redrawShips(user_data, cr);

	Simu* simu = user_data;

	drawBoard(simu->board, simu->ui.width, simu->ui.height, cr);
	/*Simu* simu = user_data;

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);
	*/

	/*g_print("==================\n");
	g_print("width = %d\n",width);
	g_print("height = %d\n",height);
	*/

	/*double rayon = 3;
	double x = 200;
	double y = 200;

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_arc(cr, x, y, rayon, 0, 2 * G_PI);
	cairo_fill(cr);
	*/

	//drawShip(simu->ship, allocation, cr);

	return FALSE;
}

gboolean on_move_ship(gpointer user_data)
{
	Simu* simu = user_data;

	int width;
	int height;

	gtk_window_get_size(simu->ui.window, &width, &height);

	for(int i = 0; i < simu->shipNumber; i++)
	{
		updateShip(simu->ship[i], width, height);
	}
	updateBoard(simu->board, width, height);

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
		return TRUE;
	}
	return FALSE;
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

	int width = 900;
	int height = 900;

	gtk_window_set_default_size(window, width, height);
	GtkDrawingArea* area = GTK_DRAWING_AREA(gtk_builder_get_object(builder,"drawing_area"));

	float shipNumber = 300;

	Simu simu =
	{
		.state = PAUSE,
		.ui =
		{
			.window = window,
			.area = area,
			.width = width,
			.height = height,
		},
		.ship = getShipsList(shipNumber, width, height),
		.shipNumber = shipNumber,
		.board = getNewBoard(width, height);
	};

	g_signal_connect(area, "draw", G_CALLBACK(on_draw), &simu);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(window, "key_press_event", G_CALLBACK(on_key_press), &simu);
	gtk_widget_show_all(GTK_WIDGET(window));

	g_object_unref(builder);

	gtk_main ();

	return 0;
}
