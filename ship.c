#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct ship {
	int x;
	int y;
	int angle;
	int speed;
};

struct ship* alloc_ship()
{
	struct ship* res = malloc(sizeof(struct ship));
	return res;
}

struct ship* getNewShip(int x, int y, int angle, int speed)
{
	struct ship* res = alloc_ship();
	res->x = x;
	res->y = y;
	if( angle < 0 || angle > 359)
	{
		printf("Error initializing ship: Angle not between 0 and 360");
	}
	res->angle = angle;
	res->speed = speed;
	return res;
}

void updateShip(struct ship* a)
{
	a->x += (int)(cos((double)(a->angle)))*a->speed;
	a->y += (int)(sin((double)(a->angle)))*a->speed;
}

void freeShip(struct ship* a)
{
	free(a);
}

