#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <stdlib.h>
#include <stdint.h>
#include "phero.h"
#include "ship.h"
#include "simu.h"

#define DIFFUSION    0.15f
#define EVAPOFACTOR  0.008f
#define MOUSE_RADIUS 25
#define MOUSE_STRENGTH 2.0f

// ─── Mouse deposit compute shader ────────────────────────────────────────────
// Attraction : result = max(current, gaussian * STRENGTH)  — never dims a trail
// Repulsion  : result = current * (1 - gaussian)           — center fully erased
static const char* CS_MOUSE_SRC =
    "#version 430 core\n"
    "layout(local_size_x = 8, local_size_y = 8) in;\n"
    "\n"
    "layout(r32f, binding = 0) uniform image2D phero;\n"
    "\n"
    "uniform int   u_cx;\n"
    "uniform int   u_cy;\n"
    "uniform int   u_radius;\n"
    "uniform float u_strength;\n"  // >0 attract, <0 repel
    "uniform int   u_width;\n"
    "uniform int   u_height;\n"
    "\n"
    "void main() {\n"
    "    ivec2 pos = ivec2(gl_GlobalInvocationID.xy)\n"
    "              + ivec2(u_cx - u_radius, u_cy - u_radius);\n"
    "    if (pos.x < 0 || pos.x >= u_width ||\n"
    "        pos.y < 0 || pos.y >= u_height) return;\n"
    "\n"
    "    float dx   = float(pos.x - u_cx);\n"
    "    float dy   = float(pos.y - u_cy);\n"
    "    float dist = sqrt(dx*dx + dy*dy);\n"
    "    if (dist > float(u_radius)) return;\n"
    "\n"
    "    float t    = dist / float(u_radius);\n"
    "    float gauss = exp(-3.0 * t * t);\n"
    "\n"
    "    float cur = imageLoad(phero, pos).r;\n"
    "    float res;\n"
    "    if (u_strength > 0.0)\n"
    "        res = max(cur, gauss * u_strength);\n"
    "    else\n"
    "        res = cur * (1.0 - gauss);\n"
    "    imageStore(phero, pos, vec4(res, 0.0, 0.0, 0.0));\n"
    "}\n";

static GLuint createMouseCS()
{
    GLuint cs = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(cs, 1, &CS_MOUSE_SRC, NULL);
    glCompileShader(cs);
    GLint ok;
    glGetShaderiv(cs, GL_COMPILE_STATUS, &ok);
    if(!ok)
    {
        char log[512];
        glGetShaderInfoLog(cs, 512, NULL, log);
        g_printerr("Mouse CS error: %s\n", log);
    }
    GLuint prog = glCreateProgram();
    glAttachShader(prog, cs);
    glLinkProgram(prog);
    glDeleteShader(cs);
    return prog;
}

// ─── Simulation ───────────────────────────────────────────────────────────────

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
        .cs_mouse   = 0,
        .board      = getNewBoard(width, height),
        .mouse_x    = 0,
        .mouse_y    = 0,
        .mouse_mode = 0,
        .stunned    = 0,
    };
    return simu;
}

void initGL(Simu* simu)
{
    simu->ui.gl_program = createRenderProgram();
    glGenVertexArrays(1, &simu->ui.gl_vao);

    initBoardGL(simu->board);

    simu->ship_ssbo = createShipSSBO(simu->ship, simu->shipNumber);
    simu->cs_ships  = createShipCS();
    simu->cs_mouse  = createMouseCS();

    for(int i = 0; i < simu->shipNumber; i++)
        freeShip(simu->ship[i]);
    free(simu->ship);
    simu->ship = NULL;
}

static void depositMousePhero(Simu* simu)
{
    PheromoneGrid* board = simu->board;
    int R = MOUSE_RADIUS;
    int D = 2 * R + 1;

    int cx = simu->mouse_x;
    int cy = simu->mouse_y;
    if (cx < R)               cx = R;
    if (cx > board->width  - 1 - R) cx = board->width  - 1 - R;
    if (cy < R)               cy = R;
    if (cy > board->height - 1 - R) cy = board->height - 1 - R;

    float strength = MOUSE_STRENGTH;

    glUseProgram(simu->cs_mouse);
    glBindImageTexture(0, board->tex[board->ping], 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glUniform1i(glGetUniformLocation(simu->cs_mouse, "u_cx"),       cx);
    glUniform1i(glGetUniformLocation(simu->cs_mouse, "u_cy"),       cy);
    glUniform1i(glGetUniformLocation(simu->cs_mouse, "u_radius"),   R);
    glUniform1f(glGetUniformLocation(simu->cs_mouse, "u_strength"), strength);
    glUniform1i(glGetUniformLocation(simu->cs_mouse, "u_width"),    board->width);
    glUniform1i(glGetUniformLocation(simu->cs_mouse, "u_height"),   board->height);

    int groups = (D + 7) / 8;
    glDispatchCompute(groups, groups, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void updateSimu(Simu* simu)
{
    PheromoneGrid* board = simu->board;
    int W = board->width;
    int H = board->height;

    // ── 1. Ship compute shader ────────────────────────────────────────────────
    static uint32_t frame = 0;
    frame++;

    glUseProgram(simu->cs_ships);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, simu->ship_ssbo);
    glBindImageTexture(1, board->tex[board->ping], 0, GL_FALSE, 0,
                       GL_READ_WRITE, GL_R32F);
    glUniform1i (glGetUniformLocation(simu->cs_ships, "u_width"),  W);
    glUniform1i (glGetUniformLocation(simu->cs_ships, "u_height"), H);
    glUniform1i (glGetUniformLocation(simu->cs_ships, "u_count"),  simu->shipNumber);
    glUniform1ui(glGetUniformLocation(simu->cs_ships, "u_seed"),    frame);
    glUniform1i (glGetUniformLocation(simu->cs_ships, "u_stunned"), simu->stunned);

    int groups = (simu->shipNumber + 63) / 64;
    glDispatchCompute(groups, 1, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // ── 2. Diffusion + évaporation ────────────────────────────────────────────
    evapoBoard(board, DIFFUSION, EVAPOFACTOR);

    // ── 3. Dépôt souris (après diffusion — reste à pleine force cette frame) ──
    if (simu->mouse_mode != 0)
        depositMousePhero(simu);
}

void redraw(gpointer user_data)
{
    Simu* simu = user_data;
    drawBoard(simu->ui.gl_program, simu->ui.gl_vao,
              simu->board->tex[simu->board->ping],
              simu->ui.width, simu->ui.height);
}

void freeSimu(Simu simu)
{
    if(simu.ship)
    {
        for(int i = 0; i < simu.shipNumber; i++)
            freeShip(simu.ship[i]);
        free(simu.ship);
    }
    glDeleteBuffers(1, &simu.ship_ssbo);
    glDeleteProgram(simu.cs_ships);
    glDeleteProgram(simu.cs_mouse);
    glDeleteVertexArrays(1, &simu.ui.gl_vao);
    glDeleteProgram(simu.ui.gl_program);
    freeBoard(simu.board);
}
