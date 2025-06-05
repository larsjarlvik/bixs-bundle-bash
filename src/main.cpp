#include <flecs.h>
#include <raylib.h>
#include <random>
#include <ctime>
#include "world/components/camera.h"
#include "world/components/rendering.h"
#include "world/world.h"

auto GetRandomFloat(float min, float max) -> float {
    return min + (((float)rand() / (float)RAND_MAX) * (max - min));
}

auto main() -> int {
    srand(static_cast<unsigned int>(time(nullptr)));

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Bix's Bundle Bash!");
    
    auto world = World::create_world();
    
    world.ecs.set<WorldCamera>({
        .camera = Camera{
            .target = {0.0F, 0.0F, 0.0F},
            .up = {0.0F, 1.0F, 0.0F},
            .fovy = 45.0F,
            .projection = CAMERA_PERSPECTIVE,
        }, 
        .distance = 5.0F
    });
    
    auto shader = LoadShader("assets/shaders/world.vs", "assets/shaders/model.fs");
    auto world_shader = WorldShader{
        .shader = shader,
        .loc_light_dir = GetShaderLocation(shader, "lightDir"),
        .loc_light_color = GetShaderLocation(shader, "lightColor"),
        .loc_view_pos = GetShaderLocation(shader, "viewPos"),
        .loc_use_texture = GetShaderLocation(shader, "useTexture"),
    };

    // Create character model
    int bix_anim_count = 0;
    
    auto bix_model = LoadModel("assets/models/bix.glb");
    ModelAnimation* bix_animations = LoadModelAnimations("assets/models/bix.glb", &bix_anim_count);

    world.ecs.entity("Bix")
        .set<WorldModel>({
            .model = bix_model, 
            .animations = bix_animations,
            .textured = true
        })
        .set<Animation>({
            .index = 1,
            .frame = 0
        })
        .set<Position>({
            .pos = {0.0F, 0.0F, 0.0F}
        })
        .set<Rotation>({
            .yaw = 0.0F
        })
        .set<WorldShader>(world_shader)
        .set<MoveTo>({
            .target = {0.0F, 0.0F, 0.0F},
            .speed = 0.05F
        });

    // Create game objects
    std::uniform_real_distribution<float> dist(-10.0F, 10.0F);
    
    auto banana_model = LoadModel("assets/models/banana.glb");
    for (int i = 0; i < 10; ++i) {
        world.ecs.entity()
            .set<WorldModel>({
                .model = banana_model,
                .textured = false
            })
            .set<Position>({
                .pos = { GetRandomFloat(-10.0F, 10.0F), 0.0F, GetRandomFloat(-10.0F, 10.0F) }
            })
            .set<WorldShader>(world_shader);
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        world.update();
        
        EndDrawing();
    }
    
    UnloadModel(bix_model);
    UnloadModel(banana_model);
    UnloadShader(shader);
    CloseWindow();
    return 0;
}
