#pragma once
#include <raylib.h>

struct InterpolationState {
    Vector3 prev_pos { 0.0F, 0.0F, 0.0F };
    Vector3 render_pos { 0.0F, 0.0F, 0.0F };
    Vector3 prev_rot { 0.0F, 0.0F, 0.0F };
    Quaternion render_rot { 0.0F, 0.0F, 0.0F };
};