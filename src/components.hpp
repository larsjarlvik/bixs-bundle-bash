#pragma once
#include <raylib.h>

struct WorldCamera {
    Camera camera;
    float distance;
};

struct WorldModel {
    Model model;
    bool textured;
};

struct WorldPos {
    Vector3 pos;
};

struct WorldShader {
    Shader shader;
    int loc_light_dir;
    int loc_light_color;
    int loc_view_pos;
    int loc_use_texture;
};
