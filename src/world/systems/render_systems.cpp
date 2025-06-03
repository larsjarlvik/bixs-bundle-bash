#include <raylib.h>
#include <flecs.h>
#include <world/components/rendering.h>
#include <world/components/camera.h>

void begin_render(flecs::iter &iter) {
    const auto *cam = iter.world().get<WorldCamera>();
    if (cam != nullptr) {
        BeginMode3D(cam->camera);
    }
}

void setup_lighting(flecs::entity entity, WorldShader &shader) {
    Vector3 light_dir = { -0.5F, -1.0F, -0.5F };
    Vector3 light_color = { 0.4F, 0.4F, 0.4F };

    SetShaderValue(shader.shader, shader.loc_light_dir, &light_dir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader.shader, shader.loc_light_color, &light_color, SHADER_UNIFORM_VEC3);
}

void render_model(flecs::entity entity, WorldModel &model, WorldShader &shader, WorldPos &pos) {
    const auto *cam = entity.world().get<WorldCamera>();
    if (cam == nullptr) {
        return;
    }
    
    int shader_bool = static_cast<int>(model.textured);
    SetShaderValue(shader.shader, shader.loc_use_texture, &shader_bool, SHADER_UNIFORM_INT);
    SetShaderValue(shader.shader, shader.loc_view_pos, &cam->camera.position, SHADER_UNIFORM_VEC3);
        
    for (int i = 0; i < model.model.materialCount; i++) {
        model.model.materials[i].shader = shader.shader;
    }
    
    DrawModel(model.model, pos.pos, 1.0F, WHITE);
}

void end_render(flecs::iter &iter) {
    EndMode3D();
}
