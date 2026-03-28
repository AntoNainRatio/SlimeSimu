#include <gtk/gtk.h>
#include <cairo.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "phero.h"

PheromoneGrid* getNewBoard(int width, int height)
{
    PheromoneGrid* grid = malloc(sizeof(PheromoneGrid));
    grid->width   = width;
    grid->height  = height;
    grid->current = calloc(width * height, sizeof(float));
    grid->next    = calloc(width * height, sizeof(float));
    return grid;
}

static void* evapo_worker(void* arg)
{
    ThreadArgs* a = (ThreadArgs*)arg;
    int W = a->grid->width;
    int H = a->grid->height;
    float diffusion   = a->diffusion;
    float evaporation = a->evaporation;

    for(int i = a->y_start; i < a->y_end; i++)
    {
        for(int j = 0; j < W; j++)
        {
            float neighbors = 0.0f;
            int count = 0;

            if(i > 0)   { neighbors += a->grid->current[(i-1)*W + j]; count++; }
            if(i < H-1) { neighbors += a->grid->current[(i+1)*W + j]; count++; }
            if(j > 0)   { neighbors += a->grid->current[i*W + (j-1)]; count++; }
            if(j < W-1) { neighbors += a->grid->current[i*W + (j+1)]; count++; }

            float avg     = (count > 0) ? neighbors / count : 0.0f;
            float val     = a->grid->current[i*W + j];
            float diffused = val * (1.0f - diffusion) + avg * diffusion;
            a->grid->next[i*W + j] = diffused * (1.0f - evaporation);
            if(a->grid->next[i*W + j] < 0.0f)
                a->grid->next[i*W + j] = 0.0f;
        }
    }
    return NULL;
}

void evapoBoard(PheromoneGrid* grid, float diffusion, float evaporation)
{
    pthread_t   threads[NB_THREADS];
    ThreadArgs  args[NB_THREADS];

    int rows_per_thread = grid->height / NB_THREADS;

    for(int t = 0; t < NB_THREADS; t++)
    {
        args[t].grid        = grid;
        args[t].diffusion   = diffusion;
        args[t].evaporation = evaporation;
        args[t].y_start     = t * rows_per_thread;
        args[t].y_end       = (t == NB_THREADS - 1)
                              ? grid->height
                              : (t + 1) * rows_per_thread;
        pthread_create(&threads[t], NULL, evapo_worker, &args[t]);
    }

    for(int t = 0; t < NB_THREADS; t++)
        pthread_join(threads[t], NULL);

    float* tmp    = grid->current;
    grid->current = grid->next;
    grid->next    = tmp;
}

void drawBoard(PheromoneGrid* grid, cairo_surface_t* surface)
{
    unsigned char* data = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);

    cairo_surface_flush(surface);

    int W = grid->width;
    int H = grid->height;

    for(int i = 0; i < H; i++)
    {
        for(int j = 0; j < W; j++)
        {
            unsigned char c = (unsigned char)(grid->current[i*W + j] * 255.0f);
            unsigned char* pixel = data + i * stride + j * 4;
            pixel[0] = c;
            pixel[1] = c;
            pixel[2] = c;
            pixel[3] = 0xFF;
        }
    }

    cairo_surface_mark_dirty(surface);
}

void freeBoard(PheromoneGrid* grid)
{
    free(grid->current);
    free(grid->next);
    free(grid);
}