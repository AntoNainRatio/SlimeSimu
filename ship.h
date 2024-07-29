#ifndef HEADER_FILE_NAME
#define HEADER_FILE_NAME

#define COTE 1
#define SPEED 2
#define PI 3.1415927
#define TURNRANGE 30

struct ship {
	float x;
	float y;
	float angle;
};

struct ship* alloc_ship();
float getNewRandomAngle(float mini, float maxi);
int getRandomPosition(int mini,int maxi);
struct ship* getNewShip(float x, float y, float angleDeg);
void handlesSides(struct ship* a, int width, int height);
void updateShip(struct ship* a, int width, int height);
void drawShip(struct ship* a, GtkAllocation allocation, cairo_t *cr);
//void redrawShip(struct ship* a, GtkAllocation allocation, cairo_t *cr);
void freeShip(struct ship* a);

#endif
