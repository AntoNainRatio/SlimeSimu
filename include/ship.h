#ifndef SHIP_H
#define SHIP_H

#include <epoxy/gl.h>

#define COTE         1
#define SPEED        1
#define PI           3.1415927f

#define SENSOR_DIST   15.0f
#define SENSOR_ANGLE  25.0f
#define SENSOR_RADIUS 2
#define TURN_DELTA_MIN  2.0f
#define TURN_DELTA_MAX  8.0f

struct ship {
    float x;
    float y;
    float angle;
};

// Layout must match the GLSL struct Ship in the compute shader (std430, 12 bytes)
typedef struct { float x, y, angle; } ShipSSBO;

struct ship* getNewShip(float x, float y, float angleDeg);
float        getNewRandomAngle(float mini, float maxi);
int          getRandomPosition(int mini, int maxi);
void         freeShip(struct ship* a);

// GPU resources
GLuint createShipSSBO(struct ship** ships, int n);
GLuint createShipCS();

#endif
