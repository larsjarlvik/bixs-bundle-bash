#pragma once
#include <raylib.h>

struct InterpolationState {
    Vector3 prev_pos { 0.0f, 0.0f, 0.0f };
    Vector3 render_pos { 0.0f, 0.0f, 0.0f };
    Vector3 prev_rot { 0.0f, 0.0f, 0.0f };
    Quaternion render_rot { 0.0f, 0.0f, 0.0f, 0.0f };
};