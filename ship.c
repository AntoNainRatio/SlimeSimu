#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <stdbool.h>
#include "ship.h"

#define ERRORPERCENT 0.02

struct ship* alloc_ship()
{
	struct ship* res = malloc(sizeof(struct ship));
	return res;
}

float getNewRandomAngle(float mini, float maxi)
{
	float degree = (rand() % (int)(maxi-mini)) + mini;
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
	res->targetAngle = angleDeg;
	res->isInCorner = 0;
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


/*void handleSides(struct ship* a, int width, int height)
{
	float ANGLETETA = absAngle(a->targetAngle - a->preAngle) * ERRORPERCENT;
	if(a->x < TURNRANGE && a->y < TURNRANGE)
	{
		a->targetAngle = 315;
		if(a->isInCorner == 0)
		{
			a->isInCorner = 1;
			a->preAngle = a->angle;
		}
		ANGLETETA = absAngle(a->targetAngle - a->preAngle) * ERRORPERCENT;
		if( a->angle <= a->targetAngle - ANGLETETA || a->angle >= a->targetAngle + ANGLETETA)
		{
			a->angle = a->angle + (a->targetAngle- a->preAngle) / 40;
		}
		else
		{
			a->targetAngle = a->angle;
		}

	}
	else if(a->x < TURNRANGE && a->y > height - TURNRANGE)
	{
		a->targetAngle = 45;
		if(a->isInCorner == 0)
		{
			a->isInCorner = 1;
			a->preAngle = a->angle;
		}
		ANGLETETA = absAngle(a->targetAngle - a->preAngle) * ERRORPERCENT;
		if( a->angle <= a->targetAngle - ANGLETETA || a->angle >= a->targetAngle + ANGLETETA)
		{
			a->angle = a->angle + (a->targetAngle- a->preAngle) / 40;
		}
		else
		{
			a->targetAngle = a->angle;
		}

	}
	else if(a->x > width - TURNRANGE && a->y < TURNRANGE)
	{
		a->targetAngle = 225;
		if(a->isInCorner == 0)
		{
			a->isInCorner = 1;
			a->preAngle = a->angle;
		}
		ANGLETETA = absAngle(a->targetAngle - a->preAngle) * ERRORPERCENT;
		if( a->angle <= a->targetAngle - ANGLETETA || a->angle >= a->targetAngle + ANGLETETA)
		{
			a->angle = a->angle + (a->targetAngle- a->preAngle) / 40;
		}
		else
		{
			a->targetAngle = a->angle;
		}

	}
	else if(a->x > width - TURNRANGE && a->y > height - TURNRANGE)
	{
		a->targetAngle = 135;
		if(a->isInCorner == 0)
		{
			a->isInCorner = 1;
			a->preAngle = a->angle;
		}
		ANGLETETA = absAngle(a->targetAngle - a->preAngle) * ERRORPERCENT;
		if( a->angle <= a->targetAngle - ANGLETETA || a->angle >= a->targetAngle + ANGLETETA)
		{
			a->angle = a->angle + (a->targetAngle- a->preAngle) / 40;
		}
		else
		{
			a->targetAngle = a->angle;
		}
	}
	else if(a->x < TURNRANGE || a->x > width - TURNRANGE)
	{
		if(a->isInCorner == 0)
		{
			a->targetAngle = getInterAngle(a->preAngle + (90 - a->preAngle)*2);
			ANGLETETA = absAngle(a->targetAngle - a->preAngle) * ERRORPERCENT;
		}
		if( a->angle <= a->targetAngle - ANGLETETA || a->angle >= a->targetAngle + ANGLETETA)
		{
			a->angle = a->angle + (a->targetAngle - a->preAngle) / 40;
		}
		else
		{
			a->targetAngle = a->angle;
		}

	}
	else if(a->y < TURNRANGE || a->y > height - TURNRANGE)
	{
		if(a->isInCorner == 0)
		{
			a->targetAngle = getInterAngle(a->preAngle + (180 - a->preAngle)*2);
			ANGLETETA = absAngle(a->targetAngle - a->preAngle) * ERRORPERCENT;
		}
		if( a->angle <= a->targetAngle - ANGLETETA || a->angle >= a->targetAngle + ANGLETETA)
		{
			if(absAngle(a->targetAngle - a->preAngle) > 180)
			{
				a->angle = getInterAngle(a->angle - (a->targetAngle - a->preAngle) / 40);
			}
			else
			{
				a->angle = getInterAngle(a->angle + (a->targetAngle - a->preAngle) / 40);
			}
		}
		else
		{
			a->targetAngle = a->angle;
		}
	}
	else
	{
		if( a->angle <= a->targetAngle - ANGLETETA || a->angle >= a->targetAngle + ANGLETETA)
		{
			a->angle = getInterAngle(a->angle + (a->targetAngle - a->preAngle) / 40);
		}
		else
		{
			a->targetAngle = a->angle;
			if(a->preAngle != a->angle)
			{
				a->preAngle = a->angle;
			}
		}
		if(a->isInCorner == 1)
		{
			a->isInCorner = 0;
		}
	}


	g_print("=============================\n");
	g_print("preAngle = %f\n",a->preAngle);
	g_print("angle = %f\n",a->angle);
	g_print("targetAngle = %f\n",a->targetAngle);
}*/

void handleSides(struct ship* a, int width, int height)
{
	if(a->x <= 0 && (a->angle > 90 && a->angle < 270))
	{
		a->angle =(int)(getNewRandomAngle(20, 160) + 180) % 360;
	}
	else if(a->x + COTE >= width && (a->angle < 90 || a->angle > 270))
	{
		a->angle = getNewRandomAngle(110, 250);
	}
	if(a->y <= 0 && (a->angle < 180 && a->angle > 0))
	{
		a->angle = getNewRandomAngle(200, 340);
	}
	else if(a->y + COTE >= height && (a->angle > 180))
	{
		a->angle = getNewRandomAngle(20, 160);
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
	cairo_rectangle(cr, a->x, a->y, COTE, COTE);
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

