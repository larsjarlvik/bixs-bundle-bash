#pragma once
#include <map>
#include <optional>
#include <raylib.h>
#include <string>

struct WorldCamera {
    Camera camera {};
    float distance {};
};

struct CameraFollow {};

struct WorldModel {
    std::map<std::string, ModelAnimation> animations {};
    Model model {};
    bool textured { false };
};

struct WorldGround {
    Model model {};
};

struct Animation {
    std::string name;
    std::optional<std::string> run_once { std::nullopt };
    float frame_time { 0.0f };
};

struct WorldTransform {
    Vector3 pos { 0.0f, 0.0f, 0.0f };
    Vector3 rot { 0.0f, 0.0f, 0.0f };
};

struct PrevWorldTransform {
    Vector3 pos { 0.0f, 0.0f, 0.0f };
    Vector3 rot { 0.0f, 0.0f, 0.0f };
};

struct ModelShader {
    Shader shader;
    int loc_light_dir;
    int loc_light_color;
    int loc_view_pos;
    int loc_use_texture;
};

struct GroundShader {
    Shader shader;
    int loc_light_dir;
    int loc_light_color;
    int loc_view_pos;
    int loc_shadow_count;
    int loc_shadow_positions;
    int loc_shadow_radii;
    int loc_shadow_itensities;
};

struct ShadowCaster {
    float radius;
};