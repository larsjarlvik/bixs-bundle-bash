#pragma once
#include <raylib.h>
#include <vector>

struct Explosion {
    int particles { 25 };
    float min_speed { 0.05f };
    float max_speed { 0.03f };
    float min_lifetime { 0.8f };
    float max_lifetime { 0.8f };
    std::vector<Color> colors {};
};

struct Particle {
    float lifetime { 0.0f };
    float variation { 1.0f };
    Color color { 0, 0, 0, 255 };
    Vector3 velocity { 0.0f, 0.0f, 0.0f };
    Vector3 rot_velocity { 0.0f, 0.0f, 0.0f };
};