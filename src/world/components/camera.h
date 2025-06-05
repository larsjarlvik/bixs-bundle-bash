#pragma once
#include <raylib.h>

struct WorldCamera {
    Camera camera;
    float distance;
};

struct MoveTo {
    Vector3 target;
    float speed;
};
