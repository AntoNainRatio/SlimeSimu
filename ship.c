#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include "ship.h"


struct ship* alloc_ship()
{
	struct ship* res = malloc(sizeof(struct ship));
	return res;
}

float getNewRandomAngle()
{
	srand(time(NULL));
	int degree = rand() % 360;
	return degree * PI / 180;;
}

struct ship* getNewShip(int x, int y, float angleRad)
{
	struct ship* res = alloc_ship();
	res->x = x;
	res->preX = x;
	res->y = y;
	res->preY = y;
	if( angleRad < 0 || angleRad > 6.26573)
	{
		printf("Error initializing ship: Angle(radian) not between 0 and 6,26573");
	}
	res->angle = angleRad;
	return res;
}

void updateShip(struct ship* a)
{
	float addx = (cos((double)(a->angle)))*SPEED;
	float tmpx = a->x + addx;
	a->preX = a->x;
	a->x = round(tmpx);
	float addy = (sin((double)(a->angle)))*SPEED;
	float tmpy = a->y - addy;
	a->preY = a->y;
	a->y = round(tmpy);
}

void drawShip(struct ship* a, GtkAllocation allocation, cairo_t *cr)
{
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_arc(cr, a->x, a->y, RAYON, 0, 2 * G_PI);
	cairo_fill(cr);
}

/*void redrawShip(struct ship* a, GtkAllocation allocation, cairo_t *cr)
{
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_arc(cr, a->preX, a->preY, RAYON, 0, 2 * G_PI);
	cairo_fill(cr);
	drawShip(a, allocation, cr);
}*/

void freeShip(struct ship* a)
{
	free(a);
}

