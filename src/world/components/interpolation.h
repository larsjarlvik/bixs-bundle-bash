#pragma once
#include <raylib.h>

struct InterpolationState {
    Vector3 prev_pos { 0.0F, 0.0F, 0.0F };
    Vector3 render_pos { 0.0F, 0.0F, 0.0F };
    float prev_yaw {};
    float render_yaw {};
};
