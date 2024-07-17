#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include <cairo.h>


#define RAYON 3
#define SPEED 5

struct ship {
	int x;
	int preX;
	int y;
	int preY;
	int angle;
};

struct ship* alloc_ship()
{
	struct ship* res = malloc(sizeof(struct ship));
	return res;
}

struct ship* getNewShip(int x, int y, int angle)
{
	struct ship* res = alloc_ship();
	res->x = x;
	res->preX = x;
	res->y = y;
	res->preY = y;
	if( angle < 0 || angle > 359)
	{
		printf("Error initializing ship: Angle not between 0 and 360");
	}
	res->angle = angle;
	return res;
}

void updateShip(struct ship* a)
{
	a->x += (int)(cos((double)(a->angle)))*SPEED;
	a->y += (int)(sin((double)(a->angle)))*SPEED;
}

void drawShip(struct ship* a, GtkAllocation allocation, cairo_t *cr)
{
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_arc(cr, a->x, a->y, RAYON, 0, 2 * G_PI);
	cairo_fill(cr);
}

void redrawShip(struct ship* a, GtkAllocation allocation, cairo_t *cr)
{
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_arc(cr, a->preX, a->preY, RAYON, 0, 2 * G_PI);
	cairo_fill(cr);
	drawShip(a, allocation, cr);
}

void freeShip(struct ship* a)
{
	free(a);
}

