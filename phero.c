#include <gtk/gtk.h>
#include <cairo.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "phero.h"

#define EVAPOFACTOR 0.025

square* getNewBoard(int width, int height)
{
	square* res = malloc(width * height * sizeof(struct square));
	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			square tmp = 
			{
				.val = 0,
				.update = 0,
			};
			res[i * width +j] = tmp;
		}
	}
	return res;
}

bool hasNotNullNeighbor(square* board, int x, int y, int width, int height)
{
	for(int i = -1; i < 2; i++)
	{
		for(int j = -1; j < 2; j++)
		{
			if(board[(y + j)*width + (x + i)].val != 0)
			{
				return true;
			}
		}
	}
	return false;
}

float getBlurValue(square* board, int x, int y, int width, int height)
{
	float res = 0.0;
	int count = 0;
	for(int i = -1; i < 2; i++)
	{
		for(int j = -1; j < 2; j++)
		{
			int newX = x + i;
			int newY = y + j;
			if(newX >= 0 && newX < width && newY >= 0 && newY < height)
			{
				res += board[(newY) * width + (newX)].val;
				count += 1;
			}
		}
	}
	return res / count;
}

void evapoBoard(square* board, int width, int height)
{
	for(int i = 0; i < width; i++)
	{
		for(int j = 0; j < height; j++)
		{
			/*if(brd[j * width + i] >= EVAPOFACTOR)
			{
				board[j * width + i] -= EVAPOFACTOR;
			}*/
			

			//float tmp = getBlurValue(board, i, j, width, height) - EVAPOFACTOR;

			float tmp = board[j * width + i].val - EVAPOFACTOR;
			if(tmp < 0)
			{
				tmp = 0;
			}
			if(tmp != board[j * width + i].val)
			{
				board[j * width + i].val = tmp;
				board[j * width + i].update = 0;
			}
		}
	}
}

void drawBoard(square* b, int width, int height, cairo_t *cr)
{
	int pixel_drawn = 0;
	for(int i = 0; i < width; i++)
	{
		for(int j = 0; j < height; j++)
		{
			if(b[j * width + i].update == 0)
			{
				float v = b[j * width + i].val;
				cairo_rectangle(cr, i, j, 1, 1);
				cairo_set_source_rgb(cr, v, v, v);
				cairo_fill(cr);
				b[j * width + i].update = 1;
				pixel_drawn += 1;
			}
		}
	}
	g_print("pixel_drawn = %d\n",pixel_drawn);
}
