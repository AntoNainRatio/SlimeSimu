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

int getRandomPosition(int mini, int maxi)
{
	return (rand()%(maxi-mini))+mini;
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
	res->preAngle = angleDeg;
	return res;
}

float degreeToRadian(int d)
{
	return d*PI/180;
}

float getInterAngle(float angle)
{
	return (int)(angle + 360) % 360;
}

float absAngle(float angle)
{
	if(angle < 0)
	{
		return -angle;
	}
	return angle;
}

void handleSides(struct ship* a, int width, int height)
{
	if(a->x < TURNRANGE || a->x > width - TURNRANGE)
	{
		float targetAngle = getInterAngle(a->preAngle + (90 - a->preAngle)*2);
		if(a->angle != targetAngle)
		{
			a->angle = a->angle + (targetAngle - a->preAngle) / 40;
		}
	}
	else if(a->y < TURNRANGE || a->y > height - TURNRANGE)
	{
		float targetAngle = getInterAngle(a->preAngle + (180 - a->preAngle)*2);
		if( a->angle != targetAngle)
		{
			if(absAngle(targetAngle - a->preAngle) > 180)
			{
				a->angle = getInterAngle(a->angle - (targetAngle - a->preAngle) / 40);
			}
			else
			{
				a->angle = getInterAngle(a->angle + (targetAngle - a->preAngle) / 40);
			}
		}
	}
	else if (a->preAngle != a->angle)
	{
		a->preAngle = a->angle;
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

