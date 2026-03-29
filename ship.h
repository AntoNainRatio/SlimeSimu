#ifndef HEADER_FILE_NAME
#define HEADER_FILE_NAME

#define COTE         1
#define SPEED        1
#define PI           3.1415927f

#define SENSOR_DIST   30.0f
#define SENSOR_ANGLE  45.0f
#define SENSOR_RADIUS 6
#define TURN_DELTA_MIN  1.0f
#define TURN_DELTA_MAX  8.0f

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