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
	int degree = rand() % 360;
	return degree;
}

struct ship* getNewShip(int x, int y, float angleDeg)
{
	struct ship* res = alloc_ship();
	res->x = x;
	res->preX = x;
	res->y = y;
	res->preY = y;
	if( angleDeg < 0 || angleDeg > 360)
	{
		printf("Error initializing ship: Angle(degree) not between 0 and 359(inlcuded)");
	}
	res->angle = angleDeg;
	return res;
}

float degreeToRadian(int d)
{
	return d*PI/180;
}

void handleSides(struct ship* a, int width, int height)
{
	if(a->x < TURNRANGE)
	{
		if(a->angle <= 180 && a->angle >= 90)
		{
			a->angle = (int)(a->angle/2);
		}
		else if(a->angle > 180 && a->angle <=270)
		{
			a->angle = (int)((a->angle+360)/2) % 360;
		}
	}

	if(a->x > width - TURNRANGE)
	{
		if(a->angle <= 90 && a->angle >= 0)
		{
			a->angle = (int)((a->angle + 180)/2);
		}
		else if(a->angle >= 270 && a->angle <= 359)
		{
			a->angle = (int)((a->angle + 180)/2);
		}
	}

	if(a->y < TURNRANGE)
	{
		if(a->angle >= 0 && a->angle <= 90)
		{
			a->angle = (int)(((a->angle+90)/2) - 90);
			if(a->angle < 0)
			{
				a->angle += 360;
			}
		}
		else if(a->angle <= 180 && a->angle > 90)
		{
			a->angle = (int)((a->angle + 270)/2);
		}
	}

	if(a->y > height - TURNRANGE)
	{
		if(a->angle <= 270 && a->angle >= 180)
		{
			a->angle = (int)((a->angle + 90 )/2);
		}
		if(a->angle >270 && a-> angle < 360)
		{
			a->angle = (int)((a->angle + 450) /2) % 360;
		}
	}
}

void updateShip(struct ship* a, int width, int height)
{
	float addx = (cos((double)(degreeToRadian(a->angle))))*SPEED;
	float tmpx = a->x + addx;
	a->preX = a->x;
	a->x = round(tmpx);

	/*g_print("x = %d\n",a->x);
	g_print("width = %d\n",width);
	*/

	float addy = (sin((double)(degreeToRadian(a->angle))))*SPEED;
	float tmpy = a->y - addy;
	a->preY = a->y;
	a->y = round(tmpy);

	/*g_print("height = %d\n",height);
	g_print("y = %d\n",a->y);
	*/

	handleSides(a,width, height);
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

