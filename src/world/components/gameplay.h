#pragma once
#include <raylib.h>
#include <vector>

struct MoveTo {
    std::vector<Vector3> path {};
    size_t waypoint = 0;
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

struct Consumer {
    float range {};
};

struct Consumable {
    std::vector<Color> colors;
    int particles { 25 };
};

struct Collider {
    float radius {};
};
