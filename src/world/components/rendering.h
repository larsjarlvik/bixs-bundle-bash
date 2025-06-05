#pragma once
#include <raylib.h>

struct WorldModel {
    Model model;
    ModelAnimation* animations;
    bool textured;
};

struct Animation {
    int index;
    int frame;
};

struct Position {
    Vector3 pos;
};

struct Rotation {
    float yaw;
};

struct WorldShader {
    Shader shader;
    int loc_light_dir;
    int loc_light_color;
    int loc_view_pos;
    int loc_use_texture;
};
