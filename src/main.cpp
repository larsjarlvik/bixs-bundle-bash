#include "components.hpp"
#include "raymath.h"
#include <flecs.h>
#include <raylib.h>

auto main() -> int {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Bix's Bundle Bash!");
    
    flecs::world ecs;
    
    ecs.set<WorldCamera>({
        .camera = Camera{
            .position = {1.0F, 1.0F, 1.0F},
            .target = {0.0F, 0.0F, 0.0F},
            .up = {0.0F, 1.0F, 0.0F},
            .fovy = 45.0F,
            .projection = CAMERA_PERSPECTIVE,
        }, 
        .distance = 5.0F
    });
    
    auto shader = LoadShader("assets/shaders/world.vs", "assets/shaders/model.fs");
    auto ecs_bix = ecs.entity("Bix")
        .set<WorldModel>({
            .model = LoadModel("assets/models/bix.glb"), 
            .textured = true
        })
        .set<WorldPos>({
            .pos = {0.0F, 0.0F, 0.0F}
        })
        .set<WorldShader>({
            .shader = shader,
            .loc_light_dir = GetShaderLocation(shader, "lightDir"),
            .loc_light_color = GetShaderLocation(shader, "lightColor"),
            .loc_view_pos = GetShaderLocation(shader, "viewPos"),
            .loc_use_texture = GetShaderLocation(shader, "useTexture"),
        });
    
    ecs.system("CameraSystem")
        .kind(flecs::OnUpdate)
        .run([](flecs::iter &iter) {
            auto *cam = iter.world().get_mut<WorldCamera>();
            
            if (!cam) {
                return;
            }
        
            cam->camera.position = Vector3Add(
                cam->camera.target,
                {cam->distance, cam->distance, cam->distance}
            );
        });
        
    ecs.system("RenderBeginSystem")
        .kind(flecs::OnUpdate)
        .run([](flecs::iter &iter) {
            const auto *cam = iter.world().get<WorldCamera>();
            if (cam) {
                BeginMode3D(cam->camera);
            }
        });

    ecs.system<WorldShader>("LightingSystem")
        .kind(flecs::OnUpdate)
        .each([](flecs::entity entity, WorldShader &shader) {
            Vector3 light_dir = { -0.5F, -1.0F, -0.5F };
            Vector3 light_color = { 0.4F, 0.4F, 0.4F };

            SetShaderValue(shader.shader, shader.loc_light_dir, &light_dir, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader.shader, shader.loc_light_color, &light_color, SHADER_UNIFORM_VEC3);
        });
        
    ecs.system<WorldModel, WorldShader, WorldPos>("ModelRenderSystem")
        .kind(flecs::OnUpdate)
        .each([](flecs::entity entity, WorldModel &model, WorldShader &shader, WorldPos &pos) {
            const auto *cam = entity.world().get<WorldCamera>();
            if (!cam) {
                return;
            }
            
            int shader_bool = static_cast<int>(model.textured);
            SetShaderValue(shader.shader, shader.loc_use_texture, &shader_bool, SHADER_UNIFORM_INT);
            SetShaderValue(shader.shader, shader.loc_view_pos, &cam->camera.position, SHADER_UNIFORM_VEC3);
                
            for (int i = 0; i < model.model.materialCount; i++) {
                model.model.materials[i].shader = shader.shader;
            }
            
            DrawModel(model.model, pos.pos, 1.0F, WHITE);
        });
            
    ecs.system("RenderEndSystem") //
        .kind(flecs::OnUpdate)
        .run([](flecs::iter &iter) { 
            EndMode3D(); 
        });
            
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        ecs.progress();
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
    