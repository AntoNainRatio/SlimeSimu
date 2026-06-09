#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <stdlib.h>
#include <math.h>
#include "ship.h"

// ─── Ship compute shader ──────────────────────────────────────────────────────
// One invocation per ship. Reads pheromone texture for sensing, steers and
// moves the ship, then deposits 1.0 at its new position.
// The same texture (binding 1) is used for both read (sensing) and write
// (deposit). Reading without coherent means invocations see last frame's
// state, which is exactly what we want.

static const char* CS_SHIPS_SRC =
    "#version 430 core\n"
    "layout(local_size_x = 64) in;\n"
    "\n"
    "struct Ship { float x; float y; float angle; };\n"
    "layout(std430, binding = 0) buffer ShipBuf { Ship ships[]; };\n"
    "layout(r32f,   binding = 1) uniform image2D phero;\n"
    "\n"
    "uniform int  u_width;\n"
    "uniform int  u_height;\n"
    "uniform int  u_count;\n"
    "uniform uint u_seed;\n"
    "uniform int  u_stunned;\n"
    "\n"
    "const float SPEED        = 1.0;\n"
    "const float SENSOR_DIST  = 15.0;\n"
    "const float SENSOR_ANGLE = 25.0;\n"
    "const int   SENSOR_R     = 2;\n"
    "const float TURN_MIN     = 2.0;\n"
    "const float TURN_MAX     = 8.0;\n"
    "const float PI           = 3.14159265;\n"
    "\n"
    "uint uhash(uint x) {\n"
    "    x ^= x >> 16u; x *= 0x45d9f3bu;\n"
    "    x ^= x >> 16u; x *= 0x45d9f3bu;\n"
    "    x ^= x >> 16u; return x;\n"
    "}\n"
    "float frand(uint s) { return float(uhash(s)) * (1.0 / 4294967295.0); }\n"
    "\n"
    "float samplePhero(vec2 c) {\n"
    "    float s = 0.0;\n"
    "    for (int dy = -SENSOR_R; dy <= SENSOR_R; dy++)\n"
    "    for (int dx = -SENSOR_R; dx <= SENSOR_R; dx++) {\n"
    "        ivec2 p = ivec2(int(c.x)+dx, int(c.y)+dy);\n"
    "        if (p.x >= 0 && p.x < u_width && p.y >= 0 && p.y < u_height)\n"
    "            s += imageLoad(phero, p).r;\n"
    "    }\n"
    "    return s;\n"
    "}\n"
    "\n"
    "float applyTurn(float cur, float delta) {\n"
    "    float d = clamp(delta, -90.0, 90.0);\n"
    "    return mod(cur + d + 360.0, 360.0);\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    uint id = gl_GlobalInvocationID.x;\n"
    "    if (int(id) >= u_count) return;\n"
    "\n"
    "    Ship s = ships[id];\n"
    "    float rad   = s.angle * PI / 180.0;\n"
    "    float rad_l = (s.angle + SENSOR_ANGLE) * PI / 180.0;\n"
    "    float rad_r = (s.angle - SENSOR_ANGLE) * PI / 180.0;\n"
    "\n"
    "    vec2 fwd = vec2(s.x + cos(rad)   * SENSOR_DIST, s.y + sin(rad)   * SENSOR_DIST);\n"
    "    vec2 lft = vec2(s.x + cos(rad_l) * SENSOR_DIST, s.y + sin(rad_l) * SENSOR_DIST);\n"
    "    vec2 rgt = vec2(s.x + cos(rad_r) * SENSOR_DIST, s.y + sin(rad_r) * SENSOR_DIST);\n"
    "\n"
    "    float f = samplePhero(fwd);\n"
    "    float l = samplePhero(lft);\n"
    "    float r = samplePhero(rgt);\n"
    "\n"
    "    uint  seed  = uhash(id ^ u_seed);\n"
    "    float delta = TURN_MIN + frand(seed) * (TURN_MAX - TURN_MIN);\n"
    "    float noise = (frand(uhash(seed + 1u)) * 2.0 - 1.0) * TURN_MIN;\n"
    "\n"
    "    if (f >= l && f >= r) { /* keep going */ }\n"
    "    else if (u_stunned == 1) {\n"
    "        float r_turn = (frand(uhash(id ^ u_seed ^ 7u)) * 2.0 - 1.0) * SENSOR_ANGLE;\n"
    "        s.angle = applyTurn(s.angle, r_turn);\n"
    "    } else if (l > f && r > f) {\n"
    "        float p_left = l / (l + r);\n"
    "        if (frand(uhash(seed + 3u)) < p_left)\n"
    "            s.angle = applyTurn(s.angle,  delta);\n"
    "        else\n"
    "            s.angle = applyTurn(s.angle, -delta);\n"
    "    } else if (l > r)           { s.angle = applyTurn(s.angle,  delta); }\n"
    "    else if (r > l)            { s.angle = applyTurn(s.angle, -delta); }\n"
    "    else                       { s.angle = applyTurn(s.angle, (frand(uhash(seed+5u)) < 0.5) ? delta : -delta); }\n"
    "\n"
    "    float move_rad = s.angle * PI / 180.0;\n"
    "    s.x += cos(move_rad) * SPEED;\n"
    "    s.y += sin(move_rad) * SPEED;\n"
    "\n"
    "    // Borders: bounce with randomised angle\n"
    "    if (s.x <= 0.0) {\n"
    "        s.x = 1.0;\n"
    "        s.angle = mod(frand(uhash(seed+2u))*160.0 - 80.0 + 360.0, 360.0);\n"
    "    } else if (s.x + 1.0 >= float(u_width)) {\n"
    "        s.x = float(u_width) - 2.0;\n"
    "        s.angle = mod(180.0 + frand(uhash(seed+3u))*160.0 - 80.0 + 360.0, 360.0);\n"
    "    }\n"
    "    if (s.y <= 0.0) {\n"
    "        s.y = 1.0;\n"
    "        s.angle = mod(90.0 + frand(uhash(seed+4u))*160.0 - 80.0 + 360.0, 360.0);\n"
    "    } else if (s.y + 1.0 >= float(u_height)) {\n"
    "        s.y = float(u_height) - 2.0;\n"
    "        s.angle = mod(270.0 + frand(uhash(seed+5u))*160.0 - 80.0 + 360.0, 360.0);\n"
    "    }\n"
    "\n"
    "    ships[id] = s;\n"
    "\n"
    "    // Deposit pheromone at new position\n"
    "    ivec2 px = clamp(ivec2(int(round(s.x)), int(round(s.y))),\n"
    "                     ivec2(0), ivec2(u_width-1, u_height-1));\n"
    "    imageStore(phero, px, vec4(1.0, 0.0, 0.0, 0.0));\n"
    "}\n";

// ─── CPU init helpers ─────────────────────────────────────────────────────────

float getNewRandomAngle(float mini, float maxi)
{
    if(maxi <= mini) return mini;
    return mini + ((float)rand() / (float)RAND_MAX) * (maxi - mini);
}

int getRandomPosition(int mini, int maxi)
{
    return (rand() % (maxi - mini)) + mini;
}

struct ship* getNewShip(float x, float y, float angleDeg)
{
    struct ship* res = malloc(sizeof(struct ship));
    res->x     = x;
    res->y     = y;
    res->angle = angleDeg;
    return res;
}

void freeShip(struct ship* a)
{
    free(a);
}

// ─── GPU resources ────────────────────────────────────────────────────────────

GLuint createShipSSBO(struct ship** ships, int n)
{
    ShipSSBO* data = malloc(n * sizeof(ShipSSBO));
    for(int i = 0; i < n; i++)
    {
        data[i].x     = ships[i]->x;
        data[i].y     = ships[i]->y;
        data[i].angle = ships[i]->angle;
    }

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * sizeof(ShipSSBO), data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    free(data);
    return ssbo;
}

GLuint createShipCS()
{
    GLuint cs = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(cs, 1, &CS_SHIPS_SRC, NULL);
    glCompileShader(cs);

    GLint ok;
    glGetShaderiv(cs, GL_COMPILE_STATUS, &ok);
    if(!ok)
    {
        char log[512];
        glGetShaderInfoLog(cs, 512, NULL, log);
        g_printerr("Ship CS error: %s\n", log);
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, cs);
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if(!ok)
    {
        char log[512];
        glGetProgramInfoLog(prog, 512, NULL, log);
        g_printerr("Ship CS link error: %s\n", log);
    }
    glDeleteShader(cs);
    return prog;
}
