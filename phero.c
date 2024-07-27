#include <stdlib.h>
#include <stdio.h>

float* getNewBoard(int width, int height)
{
	float* res = calloc(width * height, sizeof(float));
	return res;
}

void evaporBoard(float* board, int width, int height)
{
	for(int i = 0; i < width; i++)
	{
		for(int j = 0; j < height; j++)
		{
			if(board[j*width + i] >= 0.1)
			{
				board[j * width + i] -= 0.1;
			}
		}
	}
}

void drawBoard(float* b, int width, int height, cairo_t *cr)
{
	for(int i = 0; i < width; i++)
	{
		for(int j = 0; j < height; j++)
		{
			if(b[j * width + i] != 0)
			{
				cairo_set_source_rgb(cr, 1, 1, 1,b[j * width + i]);
				cairo_arc(cr, i, j, 2, 0, 2 * G_PI);
				cairo_fill(cr);
			}
		}
	}
}
