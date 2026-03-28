#ifndef PHERO_H
#define PHERO_H

#include <pthread.h>

#define NB_THREADS 14

typedef struct PheromoneGrid
{
    float* current;
    float* next;
    int width;
    int height;
} PheromoneGrid;

typedef struct ThreadArgs
{
    PheromoneGrid* grid;
    int y_start;
    int y_end;
    float diffusion;
    float evaporation;
} ThreadArgs;

PheromoneGrid* getNewBoard(int width, int height);
void evapoBoard(PheromoneGrid* grid, float diffusion, float evaporation);
void drawBoard(PheromoneGrid* grid, cairo_surface_t* surface);
void freeBoard(PheromoneGrid* grid);

#endif