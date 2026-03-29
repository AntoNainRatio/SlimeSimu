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
    if(maxi <= mini) return mini;
    return mini + ((float)rand() / (float)RAND_MAX) * (maxi - mini);
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
    // Bord gauche → forcer vers la droite : angle dans [-90, +90]
    if(round(a->x) <= 0)
    {
        a->x = 1;
        float delta = getNewRandomAngle(-80.0f, 80.0f);
        a->angle = fmodf(delta + 360.0f, 360.0f);
    }
    // Bord droit → forcer vers la gauche : angle dans [100, 260]
    else if(round(a->x) + COTE >= width)
    {
        a->x = width - COTE - 1;
        float delta = getNewRandomAngle(-80.0f, 80.0f);
        a->angle = fmodf(180.0f + delta + 360.0f, 360.0f);
    }

    // Bord haut → forcer vers le bas : angle dans [10, 170]
    if(round(a->y) <= 0)
    {
        a->y = 1;
        float delta = getNewRandomAngle(-80.0f, 80.0f);
        a->angle = fmodf(90.0f + delta + 360.0f, 360.0f);
    }
    // Bord bas → forcer vers le haut : angle dans [190, 350]
    else if(round(a->y) + COTE >= height)
    {
        a->y = height - COTE - 1;
        float delta = getNewRandomAngle(-80.0f, 80.0f);
        a->angle = fmodf(270.0f + delta + 360.0f, 360.0f);
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

// variation aleatoir entre delta_min et delta_max
static float randomTurnDelta()
{
    float range = TURN_DELTA_MAX - TURN_DELTA_MIN;
    return TURN_DELTA_MIN + ((float)rand() / (float)RAND_MAX) * range;
}

// Applique une rotation en s'assurant que le nouvel angle
// ne s'écarte pas de plus de 90° de la direction actuelle
static float applyTurn(float current_angle, float delta)
{
    float new_angle = fmodf(current_angle + delta + 360.0f, 360.0f);

    // Calcule la différence angulaire entre new_angle et current_angle
    float diff = new_angle - current_angle;
    if(diff > 180.0f)  diff -= 360.0f;
    if(diff < -180.0f) diff += 360.0f;

    // Interdit un écart de plus de 90°
    if(diff > 90.0f)
        new_angle = fmodf(current_angle + 90.0f + 360.0f, 360.0f);
    else if(diff < -90.0f)
        new_angle = fmodf(current_angle - 90.0f + 360.0f, 360.0f);

    return new_angle;
}

void steerShip(struct ship* a, float* board, int width, int height)
{
    float rad       = a->angle * PI / 180.0f;
    float rad_left  = (a->angle + SENSOR_ANGLE) * PI / 180.0f;
    float rad_right = (a->angle - SENSOR_ANGLE) * PI / 180.0f;

    float fx = a->x + cosf(rad)       * SENSOR_DIST;
    float fy = a->y + sinf(rad)       * SENSOR_DIST;
    float lx = a->x + cosf(rad_left)  * SENSOR_DIST;
    float ly = a->y + sinf(rad_left)  * SENSOR_DIST;
    float rx = a->x + cosf(rad_right) * SENSOR_DIST;
    float ry = a->y + sinf(rad_right) * SENSOR_DIST;

    float front = samplePheromones(board, width, height, fx, fy, SENSOR_RADIUS);
    float left  = samplePheromones(board, width, height, lx, ly, SENSOR_RADIUS);
    float right = samplePheromones(board, width, height, rx, ry, SENSOR_RADIUS);

    float delta = randomTurnDelta();

    if(front >= left && front >= right)
    {
        float noise = (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f) * TURN_DELTA_MIN;
        a->angle = applyTurn(a->angle, noise);
    }
    else if(left > right)
    {
        a->angle = applyTurn(a->angle, delta);
    }
    else if(right > left)
    {
        a->angle = applyTurn(a->angle, -delta);
    }
    else
    {
        float sign = (rand() % 2 == 0) ? 1.0f : -1.0f;
        a->angle = applyTurn(a->angle, sign * delta);
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

