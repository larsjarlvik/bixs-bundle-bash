#pragma once
#include <raylib.h>

struct MoveTo {
    Vector3 target { 0.0F, 0.0F, 0.0F };
    float speed {};
};

struct Spin {
    float speed {};
};

struct Bounce {
    float speed {};
    float height {};
    float elapsed {};
    float center_y {};
};