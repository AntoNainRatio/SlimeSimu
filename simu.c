#include <gtk/gtk.h>
#include <cairo.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "phero.h"
#include "ship.h"
#include "simu.h"

#define DIFFUSION   0.4f
#define EVAPOFACTOR 0.05f

struct ship** getDebugShipsList()
{
	struct ship** res = malloc(360 * sizeof(struct ship*));
	for(int i = 0; i < 360; i++)
	{
		res[i] = getNewShip(450, 450, i);
	}
	return res;
}

struct ship** getShipsList(int n, int width, int height)
{
	struct ship** res = malloc(n * sizeof(struct ship*));
	for(int i = 0; i < n; i++)
	{
		float angle = getNewRandomAngle(0, 359);
		int x = getRandomPosition(0, width);
		int y = getRandomPosition(0, height);
		struct ship* tmp = getNewShip(x, y, angle);
		res[i] = tmp;
	}
	return res;
}

Simu getNewSimu(GtkWindow* window, GtkDrawingArea* area, int width, int height, int shipNumber)
{
	Simu simu =
	{
		.state = PAUSE,
		.ui =
		{
			.window = window,
			.area = area,
			.width = width,
			.height = height,
			.firstDraw = 1,
			.surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height),
		},
		.ship = getShipsList(shipNumber, width, height),
		.shipNumber = shipNumber,
		.board = getNewBoard(width, height),
	};
	return simu;
}

void placePheroOnBoard(PheromoneGrid* grid, struct ship* ship)
{
    int x = (int)round(ship->x);
    int y = (int)round(ship->y);

    if(x < 0) x = 0;
    if(x >= grid->width)  x = grid->width  - 1;
    if(y < 0) y = 0;
    if(y >= grid->height) y = grid->height - 1;

    grid->current[y * grid->width + x] = 1.0f;
}

void updateSimu(Simu* simu)
{
    for(int i = 0; i < simu->shipNumber; i++)
    {
        updateShip(simu->ship[i], simu->board->current,
                   simu->ui.width, simu->ui.height);
        placePheroOnBoard(simu->board, simu->ship[i]);
    }
    evapoBoard(simu->board, DIFFUSION, EVAPOFACTOR);
}

void redraw(gpointer user_data, cairo_t *cr)
{
    Simu* simu = user_data;
    drawBoard(simu->board, simu->ui.surface);
    cairo_set_source_surface(cr, simu->ui.surface, 0, 0);
    cairo_paint(cr);
}

void freeSimu(Simu simu)
{
    for(int i = 0; i < simu.shipNumber; i++)
        freeShip(simu.ship[i]);
    freeBoard(simu.board);
    cairo_surface_destroy(simu.ui.surface);
}
