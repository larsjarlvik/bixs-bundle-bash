#include <flecs.h>
#include <raylib.h>
#include "world/components/gameplay.h"
#include "world/components/render.h"
#include "world/world.h"
#include "game.h"
#include "rlgl.h"
#include "util.h"
#include "world/components/particle.h"
#include "world/terrain/terrain.h"


void init_game() {
    auto world { World::create_world() };

    rlSetClipPlanes(1.0, 100.0);
    world.ecs.set<WorldCamera>({
        .camera { Camera {
            .position { 0.0f, 0.0f, 0.0f },
            .target {0.0f, 0.0f, 0.0f},
            .up {0.0f, 1.0f, 0.0f},
            .fovy = 45.0f,
            .projection = CAMERA_PERSPECTIVE,
        }},
        .distance = 3.0f
    });

    // Setup ground shader;
    const auto ground_shader{ LoadShader(ASSET_PATH("shaders/ground.vs"), ASSET_PATH("shaders/ground.fs")) };
    world.ecs.set<GroundShader>({
        .shader { ground_shader },
        .loc_light_dir { GetShaderLocation(ground_shader, "lightDir") },
        .loc_light_color { GetShaderLocation(ground_shader, "lightColor") },
        .loc_view_pos { GetShaderLocation(ground_shader, "viewPos") },
        .loc_shadow_count { GetShaderLocation(ground_shader, "shadowCount") },
        .loc_shadow_positions { GetShaderLocation(ground_shader, "shadowPositions") },
        .loc_shadow_radii { GetShaderLocation(ground_shader, "shadowRadii") },
        .loc_shadow_itensities { GetShaderLocation(ground_shader, "shadowIntensities") }
    });

    // Setup water shader
    const auto water_shader { LoadShader(ASSET_PATH("shaders/water.vs"), ASSET_PATH("shaders/water.fs")) };
    world.ecs.set<WaterShader>({
        .shader{water_shader},
        .loc_light_dir { GetShaderLocation(water_shader, "lightDir") },
        .loc_light_color { GetShaderLocation(water_shader, "lightColor") },
        .loc_view_pos { GetShaderLocation(water_shader, "viewPos") },
        .loc_time { GetShaderLocation(water_shader, "time") }
    });

    // Setup model shader
    const auto model_shader { LoadShader(ASSET_PATH("shaders/model.vs"), ASSET_PATH("shaders/model.fs")) };
    world.ecs.set<ModelShader>({
        .shader { model_shader },
        .loc_light_dir { GetShaderLocation(model_shader, "lightDir") },
        .loc_light_color { GetShaderLocation(model_shader, "lightColor") },
        .loc_view_pos { GetShaderLocation(model_shader, "viewPos") },
        .loc_use_texture { GetShaderLocation(model_shader, "useTexture") },
    });

    terrain::generate_ground(world);
    terrain::generate_water(world);

    // Create character model
    int bix_anim_count{0};

    const auto bix_model{LoadModel(ASSET_PATH("models/bix.glb"))};
    const auto *bix_animations{LoadModelAnimations(ASSET_PATH("models/bix.glb"), &bix_anim_count) };
    std::map<std::string, ModelAnimation> animationMap;

    for (int i { 0 }; i < bix_anim_count; i++) {
        animationMap[bix_animations[i].name] = bix_animations[i];
    }

    world.ecs.entity("Bix")
        .add<CameraFollow>()
        .set<WorldModel>({
            .animations { animationMap },
            .model { bix_model },
            .textured { true }
        })
        .set<Animation>({
            .name { "Idle" },
        })
        .set<WorldTransform>({})
        .set<Consumer>({ .range = 0.5f })
        .set<ShadowCaster>({ .radius = 0.5F })
        .set<MoveTo>({
            .target {0.0f, 0.0f, 0.0f},
            .speed { 0.05f }
        });

    const auto banana_model { LoadModel(ASSET_PATH("models/banana.glb")) };
    const auto banana_colors = std::vector<Color>{
        {255, 235, 59, 255},   // Bright banana yellow
        {255, 180, 15, 255},   // Vibrant orange-gold
        {240, 220, 80, 255},   // Creamy yellow
        {180, 140, 35, 255},   // Rich golden brown
        {100, 160, 60, 255},   // Banana leaf green
        {130, 80, 40, 255},    // Dark brown (stem/spots)
    };

    const auto apple_model { LoadModel(ASSET_PATH("models/apple.glb")) };
    const auto apple_colors = std::vector<Color>{
        {220, 50, 47, 255},    // Deep red (main body)
        {255, 80, 70, 255},    // Bright red (highlights)
        {180, 35, 30, 255},    // Dark red (shadows)
        {255, 140, 60, 255},   // Orange-red (gradient area)
        {255, 200, 80, 255},   // Golden yellow (bottom gradient)
        {101, 67, 33, 255}     // Brown (stem)
    };

    const auto cheese_model { LoadModel(ASSET_PATH("models/cheese.glb")) };
    const auto cheese_colors = std::vector<Color>{
       {255, 215, 0, 255},    // Bright golden yellow (main body)
        {255, 235, 60, 255},   // Vibrant cheese yellow (highlights)
        {240, 180, 20, 255},   // Rich golden orange (mid-tones)
        {200, 160, 30, 255},   // Deep cheddar gold (shadows)
        {180, 130, 15, 255},   // Aged amber (darker areas)
        {140, 100, 10, 255},   // Dark golden brown (deepest shadows)
    };

    const auto egg_model { LoadModel(ASSET_PATH("models/egg.glb")) };
    const auto egg_colors = std::vector<Color>{
        {218, 165, 32, 255},   // Golden brown (shell exterior)
        {160, 120, 45, 255},   // Darker brown (shell shadows)
        {245, 245, 240, 255},  // Off-white (shell interior/albumen)
        {255, 215, 0, 255},    // Bright yellow (yolk)
        {240, 200, 50, 255},   // Light golden yellow (yolk highlights)
        {120, 90, 35, 255},    // Deep brown (shell cracks/edges)
    };

    // Arrays for easy indexing
    const auto models = std::vector{ banana_model, apple_model, cheese_model, egg_model };
    const auto color_sets = std::vector{ banana_colors, apple_colors, cheese_colors, egg_colors };

    for (int i { 0 }; i < 100; ++i) {
        const auto consumable_type = util::GetRandomInt(0, 3);
        const auto pos = Vector3 { util::GetRandomFloat(-WORLD_SIZE, WORLD_SIZE), 0.0f, util::GetRandomFloat(-WORLD_SIZE, WORLD_SIZE) };

        if (terrain::get_height(pos.x, pos.z) < 0.1f) {
            --i;
            continue;
        }

        world.ecs.entity()
            .set<WorldModel>({ .model { models[consumable_type] } })
            .set<Spin>({ .speed { 1.0f } })
            .set<Bounce>({
                .speed { 0.05f },
                .height { 0.25f },
                .elapsed { util::GetRandomFloat(-1.0f, 1.0f) },
                .center_y { 1.0f },
            })
            .set<ShadowCaster>({ .radius = 0.15f })
            .set<WorldTransform>({
                .pos { pos },
                .rot { 0.0f, util::GetRandomFloat(0.0f, 360.0f), 0.0f }
            })
            .set<Consumable>({
                .colors = color_sets[consumable_type],
                .particles = 25,
            });
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground({ 0, 128, 179, 1 });

        if (IsKeyPressed(KEY_F)) {
            ToggleFullscreen();
        }

        world.update();

        EndDrawing();
    }

    UnloadModel(bix_model);
    UnloadModel(banana_model);
    UnloadModel(apple_model);
    UnloadShader(model_shader);
    UnloadShader(ground_shader);
    CloseWindow();
}
