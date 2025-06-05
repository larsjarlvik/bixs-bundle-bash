#pragma once
#include <raylib.h>

struct WorldCamera {
    Camera camera;
    float distance;
};

struct WorldModel {
    Model model;
    ModelAnimation* animations;
    bool textured;
};

struct Animation {
    int index;
    float frame_time;
    float speed;
};

struct WorldTransform {
    Vector3 pos;
    float yaw;
};

struct WorldShader {
    Shader shader;
    int loc_light_dir;
    int loc_light_color;
    int loc_view_pos;
    int loc_use_texture;
};
