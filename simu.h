#ifndef SIMU_H
#define SIMU_H

typedef enum State
{
	PLAY,
	PAUSE,
} State;

typedef struct UserInterface
{
	GtkWindow* window;
	GtkDrawingArea* area;
	int width;
	int height;
	int firstDraw;
	cairo_surface_t *surface;
} UserInterface;

typedef struct Simu
{
	State state;
	guint event;
	UserInterface ui;
	struct ship** ship;
	int shipNumber;
	PheromoneGrid* board;
} Simu;

struct ship** getDebugShipsList();
struct ship** getShipsList(int n, int width, int height);
Simu getNewSimu(GtkWindow* window, GtkDrawingArea* area, int width, int height, int shipNumber);
void updateSimu(Simu* simu);
void redraw(gpointer user_data, cairo_t *cr);
void freeSimu(Simu simu);

#endif
