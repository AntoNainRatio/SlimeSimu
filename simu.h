#ifndef SIMU_H
#define SIMU_H

#include <epoxy/gl.h>
#include "phero.h"
#include "ship.h"

typedef enum State
{
    PLAY,
    PAUSE,
} State;

typedef struct UserInterface
{
    GtkWindow*  window;
    GtkGLArea*  area;
    int         width;
    int         height;
    GLuint      gl_program;  // render program (fullscreen quad)
    GLuint      gl_vao;
} UserInterface;

typedef struct Simu
{
    State          state;
    guint          event;
    UserInterface  ui;
    // CPU-side ship list (populated at startup, freed after GPU upload in initGL)
    struct ship**  ship;
    int            shipNumber;
    // GPU resources
    GLuint         ship_ssbo;
    GLuint         cs_ships;
    GLuint         cs_mouse;
    PheromoneGrid* board;
    // Mouse interaction
    int            mouse_x;
    int            mouse_y;
    int            mouse_mode; // 0=none, 1=attract (left held)
    int            stunned;    // toggle: ants pick random direction each frame
} Simu;

struct ship** getShipsList(int n, int width, int height);
Simu getNewSimu(GtkWindow* window, GtkGLArea* area,
                int width, int height, int shipNumber);
void initGL(Simu* simu);
void updateSimu(Simu* simu);
void redraw(gpointer user_data);
void freeSimu(Simu simu);

#endif
