#ifndef HEADER_FILE_NAME
#define HEADER_FILE_NAME

#define COTE         1
#define SPEED        2
#define PI           3.1415927f

#define SENSOR_DIST   15.0f
#define SENSOR_ANGLE  20.0f
#define SENSOR_RADIUS 2
#define TURN_DELTA    4.0f

struct ship {
    float x;
    float y;
    float angle;
};

struct ship* alloc_ship();
float        getNewRandomAngle(float mini, float maxi);
int          getRandomPosition(int mini, int maxi);
struct ship* getNewShip(float x, float y, float angleDeg);
void         handleSides(struct ship* a, int width, int height);
float        samplePheromones(float* board, int width, int height,
                              float cx, float cy, int radius);
void         steerShip(struct ship* a, float* board, int width, int height);
void         updateShip(struct ship* a, float* board, int width, int height);
void         drawShip(struct ship* a, GtkAllocation allocation, cairo_t *cr);
void         freeShip(struct ship* a);

#endif