#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <stdlib.h>
#include <string.h>
#include "phero.h"

// ─── Diffusion compute shader ────────────────────────────────────────────────
// One invocation per pixel. Reads current pheromone texture, writes
// diffused+evaporated result to next texture.

static const char* CS_DIFFUSION_SRC =
    "#version 430 core\n"
    "layout(local_size_x = 16, local_size_y = 16) in;\n"
    "\n"
    "layout(r32f, binding = 0) readonly  uniform image2D current;\n"
    "layout(r32f, binding = 1) writeonly uniform image2D next;\n"
    "\n"
    "uniform int   u_width;\n"
    "uniform int   u_height;\n"
    "uniform float u_diffusion;\n"
    "uniform float u_evaporation;\n"
    "\n"
    "void main() {\n"
    "    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);\n"
    "    if (pos.x >= u_width || pos.y >= u_height) return;\n"
    "\n"
    "    float nb  = 0.0;\n"
    "    int   cnt = 0;\n"
    "    if (pos.y > 0)          { nb += imageLoad(current, ivec2(pos.x, pos.y-1)).r; cnt++; }\n"
    "    if (pos.y < u_height-1) { nb += imageLoad(current, ivec2(pos.x, pos.y+1)).r; cnt++; }\n"
    "    if (pos.x > 0)          { nb += imageLoad(current, ivec2(pos.x-1, pos.y)).r; cnt++; }\n"
    "    if (pos.x < u_width-1)  { nb += imageLoad(current, ivec2(pos.x+1, pos.y)).r; cnt++; }\n"
    "\n"
    "    float avg = (cnt > 0) ? nb / float(cnt) : 0.0;\n"
    "    float val = imageLoad(current, pos).r;\n"
    "    float res = max(0.0, (val*(1.0-u_diffusion) + avg*u_diffusion) * (1.0-u_evaporation));\n"
    "    imageStore(next, pos, vec4(res, 0.0, 0.0, 0.0));\n"
    "}\n";

// ─── Render shaders (vertex + fragment) ───────────────────────────────────────

static const char* VERT_SRC =
    "#version 330 core\n"
    "const vec2 pos[4] = vec2[](\n"
    "    vec2(-1.0, -1.0),\n"
    "    vec2( 1.0, -1.0),\n"
    "    vec2(-1.0,  1.0),\n"
    "    vec2( 1.0,  1.0)\n"
    ");\n"
    "const vec2 uvs[4] = vec2[](\n"
    "    vec2(0.0, 1.0),\n"
    "    vec2(1.0, 1.0),\n"
    "    vec2(0.0, 0.0),\n"
    "    vec2(1.0, 0.0)\n"
    ");\n"
    "out vec2 uv;\n"
    "void main() {\n"
    "    uv = uvs[gl_VertexID];\n"
    "    gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);\n"
    "}\n";

static const char* FRAG_SRC =
    "#version 330 core\n"
    "in vec2 uv;\n"
    "out vec4 color;\n"
    "uniform sampler2D pheromones;\n"
    "void main() {\n"
    "    float v = texture(pheromones, uv).r;\n"
    "    color = vec4(v, v, v, 1.0);\n"
    "}\n";

// ─── Helpers ──────────────────────────────────────────────────────────────────

static GLuint compileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if(!ok)
    {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        g_printerr("Shader error: %s\n", log);
    }
    return shader;
}

static GLuint linkProgram(GLuint* shaders, int count)
{
    GLuint prog = glCreateProgram();
    for(int i = 0; i < count; i++) glAttachShader(prog, shaders[i]);
    glLinkProgram(prog);
    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if(!ok)
    {
        char log[512];
        glGetProgramInfoLog(prog, 512, NULL, log);
        g_printerr("Program link error: %s\n", log);
    }
    for(int i = 0; i < count; i++) glDeleteShader(shaders[i]);
    return prog;
}

// ─── Public API ───────────────────────────────────────────────────────────────

PheromoneGrid* getNewBoard(int width, int height)
{
    PheromoneGrid* grid = malloc(sizeof(PheromoneGrid));
    grid->width        = width;
    grid->height       = height;
    grid->tex[0]       = 0;
    grid->tex[1]       = 0;
    grid->ping         = 0;
    grid->cs_diffusion = 0;
    return grid;
}

void initBoardGL(PheromoneGrid* grid)
{
    // Create two ping-pong R32F textures, cleared to 0
    float* zeros = calloc(grid->width * grid->height, sizeof(float));
    for(int i = 0; i < 2; i++)
    {
        glGenTextures(1, &grid->tex[i]);
        glBindTexture(GL_TEXTURE_2D, grid->tex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid->width, grid->height,
                     0, GL_RED, GL_FLOAT, zeros);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    free(zeros);

    // Compile diffusion compute shader
    GLuint cs = compileShader(GL_COMPUTE_SHADER, CS_DIFFUSION_SRC);
    grid->cs_diffusion = linkProgram(&cs, 1);
}

void evapoBoard(PheromoneGrid* grid, float diffusion, float evaporation)
{
    int next = 1 - grid->ping;

    glUseProgram(grid->cs_diffusion);
    glBindImageTexture(0, grid->tex[grid->ping], 0, GL_FALSE, 0, GL_READ_ONLY,  GL_R32F);
    glBindImageTexture(1, grid->tex[next],       0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    glUniform1i(glGetUniformLocation(grid->cs_diffusion, "u_width"),       grid->width);
    glUniform1i(glGetUniformLocation(grid->cs_diffusion, "u_height"),      grid->height);
    glUniform1f(glGetUniformLocation(grid->cs_diffusion, "u_diffusion"),   diffusion);
    glUniform1f(glGetUniformLocation(grid->cs_diffusion, "u_evaporation"), evaporation);

    int gx = (grid->width  + 15) / 16;
    int gy = (grid->height + 15) / 16;
    glDispatchCompute(gx, gy, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    grid->ping = next;
}

void freeBoard(PheromoneGrid* grid)
{
    glDeleteTextures(2, grid->tex);
    glDeleteProgram(grid->cs_diffusion);
    free(grid);
}

GLuint createRenderProgram()
{
    GLuint shaders[2] = {
        compileShader(GL_VERTEX_SHADER,   VERT_SRC),
        compileShader(GL_FRAGMENT_SHADER, FRAG_SRC),
    };
    return linkProgram(shaders, 2);
}

void drawBoard(GLuint program, GLuint vao, GLuint texture, int width, int height)
{
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(program, "pheromones"), 0);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
