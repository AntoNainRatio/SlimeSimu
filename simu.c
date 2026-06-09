#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <stdlib.h>
#include <stdint.h>
#include "phero.h"
#include "ship.h"
#include "simu.h"

#define DIFFUSION   0.15f
#define EVAPOFACTOR 0.008f

struct ship** getShipsList(int n, int width, int height)
{
    struct ship** res = malloc(n * sizeof(struct ship*));
    for(int i = 0; i < n; i++)
    {
        float angle = getNewRandomAngle(0, 359);
        int   x     = getRandomPosition(0, width);
        int   y     = getRandomPosition(0, height);
        res[i] = getNewShip(x, y, angle);
    }
    return res;
}

Simu getNewSimu(GtkWindow* window, GtkGLArea* area,
                int width, int height, int shipNumber)
{
    Simu simu =
    {
        .state      = PAUSE,
        .ui =
        {
            .window     = window,
            .area       = area,
            .width      = width,
            .height     = height,
            .gl_program = 0,
            .gl_vao     = 0,
        },
        .ship       = getShipsList(shipNumber, width, height),
        .shipNumber = shipNumber,
        .ship_ssbo  = 0,
        .cs_ships   = 0,
        .board      = getNewBoard(width, height),
    };
    return simu;
}

// Called once after the GL context is ready
void initGL(Simu* simu)
{
    // Render pipeline
    simu->ui.gl_program = createRenderProgram();
    glGenVertexArrays(1, &simu->ui.gl_vao);

    // Pheromone GPU textures + diffusion compute shader
    initBoardGL(simu->board);

    // Ship SSBO + ship compute shader
    simu->ship_ssbo = createShipSSBO(simu->ship, simu->shipNumber);
    simu->cs_ships  = createShipCS();

    // CPU ship data no longer needed
    for(int i = 0; i < simu->shipNumber; i++)
        freeShip(simu->ship[i]);
    free(simu->ship);
    simu->ship = NULL;
}

void updateSimu(Simu* simu)
{
    PheromoneGrid* board = simu->board;
    int W = board->width;
    int H = board->height;

    // ── 1. Ship compute shader ────────────────────────────────────────────────
    // Reads pheromone texture for sensing, updates SSBO positions, deposits 1.0.
    static uint32_t frame = 0;
    frame++;

    glUseProgram(simu->cs_ships);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, simu->ship_ssbo);
    glBindImageTexture(1, board->tex[board->ping], 0, GL_FALSE, 0,
                       GL_READ_WRITE, GL_R32F);
    glUniform1i (glGetUniformLocation(simu->cs_ships, "u_width"),  W);
    glUniform1i (glGetUniformLocation(simu->cs_ships, "u_height"), H);
    glUniform1i (glGetUniformLocation(simu->cs_ships, "u_count"),  simu->shipNumber);
    glUniform1ui(glGetUniformLocation(simu->cs_ships, "u_seed"),   frame);

    int groups = (simu->shipNumber + 63) / 64;
    glDispatchCompute(groups, 1, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // ── 2. Diffusion compute shader ───────────────────────────────────────────
    // Reads tex[ping] (pheromones + fresh deposits), writes diffused result to
    // tex[1-ping], then swaps ping.
    evapoBoard(board, DIFFUSION, EVAPOFACTOR);
}

void redraw(gpointer user_data)
{
    Simu* simu = user_data;
    // Pheromones are already on GPU — just bind and draw
    drawBoard(simu->ui.gl_program, simu->ui.gl_vao,
              simu->board->tex[simu->board->ping],
              simu->ui.width, simu->ui.height);
}

void freeSimu(Simu simu)
{
    // CPU ships may still exist if initGL was never called
    if(simu.ship)
    {
        for(int i = 0; i < simu.shipNumber; i++)
            freeShip(simu.ship[i]);
        free(simu.ship);
    }
    glDeleteBuffers(1, &simu.ship_ssbo);
    glDeleteProgram(simu.cs_ships);
    glDeleteVertexArrays(1, &simu.ui.gl_vao);
    glDeleteProgram(simu.ui.gl_program);
    freeBoard(simu.board);
}
