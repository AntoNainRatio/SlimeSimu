#ifndef HEADER_FILE_NAME
#define HEADER_FILE_NAME

#define RAYON 3
#define SPEED 2
#define PI 3.1415927

struct ship {
	int x;
	int preX;
	int y;
	int preY;
	float angle;
};

struct ship* alloc_ship();
float getNewRandomAngle();
struct ship* getNewShip(int x, int y, float angleRad);
void updateShip(struct ship* a);
void drawShip(struct ship* a, GtkAllocation allocation, cairo_t *cr);
//void redrawShip(struct ship* a, GtkAllocation allocation, cairo_t *cr);
void freeShip(struct ship* a);

#endif
