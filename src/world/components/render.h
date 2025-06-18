#pragma once
#include <map>
#include <raylib.h>
#include <string>

struct WorldCamera {
    Camera camera;
    float distance;
};

struct CameraFollow {};

struct WorldModel {
    Model model;
    std::map<std::string, ModelAnimation> animations;
    bool textured;
};

struct Animation {
    std::string name;
    float frame_time;
};

struct WorldTransform {
    Vector3 pos;
    float yaw;
};

struct PrevWorldTransform {
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
