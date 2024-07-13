#ifndef HEADER_FILE_NAME
#define HEADER_FILE_NAME

struct ship {
	int x;
	int y;
	int angle;
	int speed;
};

struct ship* alloc_ship();
struct ship* getNewShip(int x, int y, int angle, int speed);
void updateShip(struct ship* a);
void freeShip(struct ship* a);

#endif
