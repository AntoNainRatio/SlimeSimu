#ifndef HEADER_FILE_NAME
#define HEADER_FILE_NAME

#define RAYON 3
#define SPEED 2
#define PI 3.1415927
#define TURNRANGE 30

struct ship {
	int x;
	int preX;
	int y;
	int preY;
	float angle;
	float preAngle;
};

struct ship* alloc_ship();
float getNewRandomAngle();
int getRandomPosition(int mini,int maxi);
struct ship* getNewShip(int x, int y, float angleDeg);
void handlesSides(struct ship* a, int width, int height);
void updateShip(struct ship* a, int width, int height);
void drawShip(struct ship* a, GtkAllocation allocation, cairo_t *cr);
//void redrawShip(struct ship* a, GtkAllocation allocation, cairo_t *cr);
void freeShip(struct ship* a);

#endif
