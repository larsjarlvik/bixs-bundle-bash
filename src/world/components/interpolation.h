#pragma once
#include <raylib.h>

struct InterpolationState {
    Vector3 prev_pos, render_pos;
    float prev_yaw, render_yaw;
};
