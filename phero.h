#ifndef PHERO_H
#define PHERO_H

typedef struct square
{
	float val;
	int update;
} square;

square* getNewBoard(int width, int height);
void evapoBoard(square* board, int width, int height);
void drawBoard(square* board, int width, int height, cairo_t *cr);

#endif
