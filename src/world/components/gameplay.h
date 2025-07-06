#pragma once
#include <raylib.h>
#include <vector>

struct MoveTo {
    Vector3 target { 0.0f, 0.0f, 0.0f };
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
