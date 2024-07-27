#ifndef PHERO_H
#define PHERO_H

float* getNewBoard(int width, int height);
void evapoBoard(float* board, int width, int height);
void drawBoard(float* board, int width, int height, GtkAllocation allocation, cairo_t *cr);

#endif
