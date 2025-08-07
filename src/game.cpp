#include <flecs.h>
#include <raylib.h>
#include "world/components/gameplay.h"
#include "world/components/render.h"
#include "world/world.h"
#include "game.h"
#include "rlgl.h"
#include "util.h"
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
        .set<WorldTransform>({
            .pos = { 0.0f,  terrain::get_height(0.0f, 0.0f), 0.0f },
        })
        .set<Consumer>({ .range = 0.5f })
        .set<ShadowCaster>({ .radius = 0.5F })
        .set<MoveTo>({
            .speed { 0.05f }
        });

    const auto tree_models = std::vector{
        LoadModel(ASSET_PATH("models/tree-1.glb")),
        LoadModel(ASSET_PATH("models/tree-2.glb")),
    };

    // Configure mipmapping and filtering for all materials in all models
    for (const auto& tree_model : tree_models) {
        for (int i = 0; i < tree_model.materialCount; i++) {
            if (const auto& material = tree_model.materials[i]; material.maps[MATERIAL_MAP_DIFFUSE].texture.id > 0) {
                auto& texture = material.maps[MATERIAL_MAP_DIFFUSE].texture;
                GenTextureMipmaps(&texture);
                SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
            }
        }
    }

    for (int i { 0 }; i < 20; ++i) {
        const auto size = util::GetRandomFloat(0.9f, 1.3f);
        auto pos = Vector3 { util::GetRandomFloat(-WORLD_SIZE, WORLD_SIZE), 0.0f, util::GetRandomFloat(-WORLD_SIZE, WORLD_SIZE) };
        pos.y = terrain::get_height(pos.x, pos.z);

        if (pos.y < 0.5f) {
            --i;
            continue;
        }

        const auto tree_type = util::GetRandomInt(0, static_cast<int>(tree_models.size() - 1));
        const auto model = tree_models[tree_type];
        world.ecs.entity()
            .set<WorldModel>({ .model { model }, .textured = true })
            .set<ShadowCaster>({ .radius = 1.0f * size })
            .set<Collider>({ .radius = 0.5f })
            .set<WorldTransform>({
                .pos { pos },
                .rot { 0.0f, util::GetRandomFloat(0.0f, 360.0f), 0.0f },
                .scale { size }
            });
    }

    const auto banana_model { LoadModel(ASSET_PATH("models/banana.glb")) };
    const auto banana_colors = std::vector<Color>{
        {255, 255, 0, 255},    // Electric banana yellow
        {255, 165, 0, 255},    // Blazing orange-gold
        {255, 240, 0, 255},    // Neon creamy yellow
        {200, 140, 0, 255},    // Intense golden brown
        {50, 205, 50, 255},    // Vivid lime green
        {139, 69, 19, 255},    // Rich saddle brown
    };

    const auto apple_model { LoadModel(ASSET_PATH("models/apple.glb")) };
    const auto apple_colors = std::vector<Color>{
        {255, 0, 0, 255},      // Pure crimson red
        {255, 69, 0, 255},     // Orange-red flame
        {178, 34, 34, 255},    // Fire brick red
        {255, 140, 0, 255},    // Dark orange burst
        {255, 215, 0, 255},    // Gold highlight
        {160, 82, 45, 255}     // Saddle brown stem
    };

    const auto cheese_model { LoadModel(ASSET_PATH("models/cheese.glb")) };
    const auto cheese_colors = std::vector<Color>{
        {255, 215, 0, 255},    // Pure gold
        {255, 255, 0, 255},    // Electric yellow
        {255, 140, 0, 255},    // Dark orange
        {218, 165, 32, 255},   // Goldenrod
        {184, 134, 11, 255},   // Dark goldenrod
        {139, 69, 19, 255},    // Saddle brown depths
    };

    const auto egg_model { LoadModel(ASSET_PATH("models/egg.glb")) };
    const auto egg_colors = std::vector<Color>{
        {139, 69, 19, 255},    // Rich saddle brown shell
        {160, 82, 45, 255},    // Saddle brown shadows
        {210, 180, 140, 255},  // Warm tan shell
        {255, 255, 0, 255},    // Electric yolk yellow
        {255, 215, 0, 255},    // Gold yolk highlights
        {101, 67, 33, 255},    // Dark olive brown cracks
    };

    const auto ice_cream_model { LoadModel(ASSET_PATH("models/ice-cream.glb")) };
    const auto ice_cream_colors = std::vector<Color>{
        {138, 43, 226, 255},   // Purple (top scoop)
        {220, 20, 60, 255},    // Crimson red (middle scoop)
        {255, 140, 0, 255},    // Dark orange (cone)
        {160, 82, 45, 255},    // Saddle brown (cone shadow)
        {75, 0, 130, 255},     // Indigo (purple variation)
        {178, 34, 34, 255},    // Fire brick red (red variation)
    };

    // Arrays for easy indexing
    const auto models = std::vector{ banana_model, apple_model, cheese_model, egg_model, ice_cream_model };
    const auto color_sets = std::vector{ banana_colors, apple_colors, cheese_colors, egg_colors, ice_cream_colors};

    for (int i { 0 }; i < 100; ++i) {
        const auto consumable_type = util::GetRandomInt(0, static_cast<int>(models.size()) - 1);
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
            .set<ShadowCaster>({ .radius = 0.1f })
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
