#include <gtk/gtk.h>
#include <cairo.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "phero.h"
#include "ship.h"
#include "simu.h"

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

void placePheroOnBoard(struct square* board, struct ship* ship, int width, int height)
{
	board[(int)(round(ship->y) * width + round(ship->x))].val = 1.0;
	for(int i = -1; i < 2; i++)
	{
		for(int j = -1; j < 2; j++)
		{
			int newX = (int)(round(ship->x + j));
			int newY = (int)(round(ship->y + i));
			if(newX >= 0 && newX < width && newY >= 0 && newY < height)
			{
				board[newY * width + newX].neighbor = 1;
			}
		}
	}
}

void updateSimu(Simu* simu)
{
	for(int i = 0; i < simu->shipNumber; i++)
	{
		updateShip(simu->ship[i], simu->ui.width, simu->ui.height);
		placePheroOnBoard(simu->board, simu->ship[i], simu->ui.width, simu->ui.height);
	}
	evapoBoard(simu->board, simu->ui.width, simu->ui.height);
}

void redraw(gpointer user_data, cairo_t *cr)
{
	Simu* simu = user_data;
	//cairo_set_source_rgb(cr, 0, 0, 0);
	//cairo_paint(cr);


	/*cairo_set_source_rgb(cr, 1, 1, 1);
	for(int i = 0; i < simu->shipNumber; i++)
	{
		cairo_rectangle(cr, round(simu->ship[i]->x), round(simu->ship[i]->y), COTE, COTE);
	}
	cairo_fill(cr);*/

	drawBoard(simu->board, simu->ui.width, simu->ui.height, cr);
}

void freeSimu(Simu simu)
{
	for(int i = 0; i < simu.shipNumber; i++)
	{
		freeShip(simu.ship[i]);
	}
	cairo_surface_destroy(simu.ui.surface);
	free(simu.board);
}
