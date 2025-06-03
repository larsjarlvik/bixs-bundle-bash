#include <flecs.h>
#include <raylib.h>
#include <world/components/camera.h>
#include <world/components/rendering.h>
#include <world/world.h>

auto main() -> int {
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
    auto ecs_bix = world.ecs.entity("Bix")
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


    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        world.update();
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
