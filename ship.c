#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <stdbool.h>
#include "ship.h"

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

struct ship* getNewShip(float x, float y, float angleDeg)
{
	struct ship* res = alloc_ship();
	res->x = x;
	res->y = y;
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
	if(round(a->x) <= 0 && (a->angle > 90 && a->angle < 270))
	{
		a->angle = (rand()%160+280) % 360;
	}
	else if(round(a->x) + COTE >= width && (a->angle < 90 || a->angle > 270))
	{
		a->angle = getNewRandomAngle(110, 250);
	}
	if(round(a->y) <= 0 && (a->angle > 180))
	{
		a->angle = getNewRandomAngle(10, 170);
	}
	else if(round(a->y) + COTE >= height  && (a->angle < 180))
	{
		a->angle = getNewRandomAngle(190, 350);
	}
}

// Somme les phéromones dans un cercle de rayon `radius` et de centre (cx, cy)
float samplePheromones(float* board, int width, int height,
                       float cx, float cy, int radius)
{
    float sum = 0.0f;
    int x0 = (int)cx - radius;
    int x1 = (int)cx + radius;
    int y0 = (int)cy - radius;
    int y1 = (int)cy + radius;

    for(int y = y0; y <= y1; y++)
    {
        for(int x = x0; x <= x1; x++)
        {
            if(x >= 0 && x < width && y >= 0 && y < height)
                sum += board[y * width + x];
        }
    }
    return sum;
}

void steerShip(struct ship* a, float* board, int width, int height)
{
    float rad      = a->angle * PI / 180.0f;
    float rad_left = (a->angle + SENSOR_ANGLE) * PI / 180.0f;
    float rad_right= (a->angle - SENSOR_ANGLE) * PI / 180.0f;

    // Position des 3 capteurs
    float fx = a->x + cos(rad)       * SENSOR_DIST;
    float fy = a->y + sin(rad)       * SENSOR_DIST;
    float lx = a->x + cos(rad_left)  * SENSOR_DIST;
    float ly = a->y + sin(rad_left)  * SENSOR_DIST;
    float rx = a->x + cos(rad_right) * SENSOR_DIST;
    float ry = a->y + sin(rad_right) * SENSOR_DIST;

    float front = samplePheromones(board, width, height, fx, fy, SENSOR_RADIUS);
    float left  = samplePheromones(board, width, height, lx, ly, SENSOR_RADIUS);
    float right = samplePheromones(board, width, height, rx, ry, SENSOR_RADIUS);

    if(front >= left && front >= right)
    {
        // Tout droit — pas de changement
    }
    else if(left > right)
    {
        a->angle = fmodf(a->angle + TURN_DELTA + 360.0f, 360.0f);
    }
    else if(right > left)
    {
        a->angle = fmodf(a->angle - TURN_DELTA + 360.0f, 360.0f);
    }
    else
    {
        // Égalité gauche/droite — petit bruit aléatoire pour éviter les blocages
        float noise = (rand() % 2 == 0) ? TURN_DELTA : -TURN_DELTA;
        a->angle = fmodf(a->angle + noise + 360.0f, 360.0f);
    }
}

void updateShip(struct ship* a, float* board, int width, int height)
{
    steerShip(a, board, width, height);

    float addx = cosf(a->angle * PI / 180.0f) * SPEED;
    float addy = sinf(a->angle * PI / 180.0f) * SPEED;
    a->x += addx;
    a->y += addy;

    handleSides(a, width, height);
}

void drawShip(struct ship* a, GtkAllocation allocation, cairo_t *cr)
{
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_rectangle(cr, round(a->x), round(a->y), COTE, COTE);
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

