#ifndef PHERO_H
#define PHERO_H

#include <epoxy/gl.h>

typedef struct PheromoneGrid
{
    int    width, height;
    GLuint tex[2];        // ping-pong textures (GL_R32F)
    int    ping;          // index of the current (most recent) texture
    GLuint cs_diffusion;
} PheromoneGrid;

PheromoneGrid* getNewBoard(int width, int height);
void           initBoardGL(PheromoneGrid* grid);
void           evapoBoard(PheromoneGrid* grid, float diffusion, float evaporation);
void           freeBoard(PheromoneGrid* grid);

// Render pipeline
GLuint createRenderProgram();
void   drawBoard(GLuint program, GLuint vao, GLuint texture, int width, int height);

#endif
