#pragma once
#include <raylib.h>
#include <vector>

struct Explosion {
    int particles { 25 };
    float min_speed { 0.01F };
    float max_speed { 0.03F };
    float min_lifetime { 0.8F };
    float max_lifetime { 0.8F };
    std::vector<Color> colors;
};

struct Particle {
    float lifetime { 0.0F };
    float variation { 1.0F };
    Color color { 0, 0, 0, 255 };
    Vector3 velocity { 0.0F, 0.0F, 0.0F };
    Vector3 rot_velocity { 0.0F, 0.0F, 0.0F };
};