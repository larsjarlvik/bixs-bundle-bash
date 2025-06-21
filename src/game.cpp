#include <flecs.h>
#include <raylib.h>
#include <random>
#include <ctime>
#include "world/components/gameplay.h"
#include "world/components/render.h"
#include "world/world.h"
#include "game.h"
#include "rlgl.h"

#ifdef PLATFORM_ANDROID
    #define ASSET_PATH(path) path
#else
    #define ASSET_PATH(path) "assets/" path
#endif


auto GetRandomFloat(const float min, const float max) -> float {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_real_distribution dist(min, max);
    return dist(gen);
}

void init_game() {
    auto world { World::create_world() };

    rlSetClipPlanes(1.0, 100.0);
    world.ecs.set<WorldCamera>({
        .camera { Camera {
            .target {0.0F, 0.0F, 0.0F},
            .up {0.0F, 1.0F, 0.0F},
            .fovy  { 45.0F },
            .projection { CAMERA_PERSPECTIVE },
        }},
        .distance { 3.0F }
    });

    const auto shader { LoadShader(ASSET_PATH("shaders/world.vs"), ASSET_PATH("shaders/model.fs")) };
    const auto world_shader { WorldShader{
        .shader { shader },
        .loc_light_dir { GetShaderLocation(shader, "lightDir") },
        .loc_light_color { GetShaderLocation(shader, "lightColor") },
        .loc_view_pos { GetShaderLocation(shader, "viewPos") },
        .loc_use_texture { GetShaderLocation(shader, "useTexture") },
    }};

    // Create character model
    int bix_anim_count { 0 };

    const auto bix_model { LoadModel(ASSET_PATH("models/bix.glb")) };
    const auto* bix_animations { LoadModelAnimations(ASSET_PATH("models/bix.glb"), &bix_anim_count) };
    std::map<std::string, ModelAnimation> animationMap;

    for (int i { 0 }; i < bix_anim_count; i++) {
        animationMap[bix_animations[i].name] = bix_animations[i];
    }

    world.ecs.entity("Bix")
        .add<CameraFollow>()
        .set<WorldModel>({
            .model { bix_model },
            .animations { animationMap },
            .textured { true }
        })
        .set<Animation>({
            .name { "Idle" },
            .frame_time { 0 },
        })
        .set<WorldTransform>({
            .pos  {0.0F, 0.0F, 0.0F},
            .yaw { 0.0F }
        })
        .set<WorldShader>(world_shader)
        .set<MoveTo>({
            .target {0.0F, 0.0F, 0.0F},
            .speed { 0.05F }
        });

    const auto banana_model { LoadModel(ASSET_PATH("models/banana.glb")) };
    for (int i { 0 }; i < 10; ++i) {
        // ReSharper disable once CppExpressionWithoutSideEffects
        world.ecs.entity()
            .set<WorldModel>({ .model { banana_model } })
            .set<Spin>({ .speed { 1.0F } })
            .set<Bounce>({
                .speed { 0.05F },
                .height { 0.2F },
                .center_y = { 1.0F },
                .elapsed { GetRandomFloat(-1.0F, 1.0F) }
            })
            .set<WorldTransform>({
                .pos { GetRandomFloat(-5.0F, 5.0F), 2.0F, GetRandomFloat(-5.0F, 5.0F) },
                .yaw { GetRandomFloat(0.0F, 360.0F) }
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
}
