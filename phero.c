#include <gtk/gtk.h>
#include <cairo.h>
#include <stdlib.h>
#include <stdio.h>

#define EVAPOFACTOR 0.025

float* getNewBoard(int width, int height)
{
	float* res = calloc(width * height, sizeof(float));
	return res;
}

void evapoBoard(float* board, int width, int height)
{
	for(int i = 0; i < width; i++)
	{
		for(int j = 0; j < height; j++)
		{
			if(board[j*width + i] >= EVAPOFACTOR)
			{
				board[j * width + i] -= EVAPOFACTOR;
			}
			else
			{
				board[j * width + i] = 0;
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
				cairo_set_source_rgba(cr, 1, 1, 1,b[j * width + i]);
				cairo_rectangle(cr, i, j, 1, 1);
				cairo_fill(cr);
			}
		}
	}
}
