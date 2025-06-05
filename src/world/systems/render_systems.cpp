#include <raylib.h>
#include <flecs.h>
#include <raymath.h>
#include "world/components/rendering.h"
#include "world/components/camera.h"

namespace render_systems {
    void begin_render(flecs::iter &iter) {
        while (iter.next()) {
            const auto *cam = iter.world().get<WorldCamera>();
            BeginMode3D(cam->camera);
        }
    }

    void setup_lighting(flecs::entity entity, WorldShader &shader) {
        Vector3 light_dir = { -0.5F, -1.0F, -0.5F };
        Vector3 light_color = { 0.4F, 0.4F, 0.4F };

        SetShaderValue(shader.shader, shader.loc_light_dir, &light_dir, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader.shader, shader.loc_light_color, &light_color, SHADER_UNIFORM_VEC3);
    }

    void animate_model(flecs::entity entity, WorldModel &model, Animation &anim) {
        if (model.animations == nullptr || model.animations[anim.index].frameCount <= 0) {
            return;
        }

        UpdateModelAnimation(model.model, model.animations[anim.index], anim.frame);
        anim.frame++;
    }

    void render_model(flecs::entity entity, WorldModel &model, WorldShader &shader, Position &pos, const Rotation* rot) {
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
        
        Matrix mat_rotation = (rot != nullptr) ? QuaternionToMatrix(QuaternionFromAxisAngle((Vector3){ 0, 1, 0 }, DEG2RAD * rot->yaw)) : MatrixIdentity();
        Matrix mat_translation = MatrixTranslate(pos.pos.x, pos.pos.y, pos.pos.z);
        Matrix mat_transform = MatrixMultiply(mat_rotation, mat_translation);
        
        model.model.transform = mat_transform;

        DrawModel(model.model, {0}, 1.0F, WHITE);
    }

    void end_render(flecs::iter &iter) {
        EndMode3D();
    }

    void register_systems(flecs::world &ecs) {
        ecs.system("begin_render")
            .kind(flecs::OnUpdate)
            .with<WorldCamera>()
            .run(begin_render);

        ecs.system<WorldShader>("setup_lighting")
            .kind(flecs::OnUpdate)
            .each(setup_lighting);

        ecs.system<WorldModel, Animation>("animate_model") 
            .kind(flecs::OnUpdate)
            .each(animate_model);
            
        ecs.system<WorldModel, WorldShader, Position, const Rotation*>("render_model")
            .kind(flecs::OnUpdate)
            .each(render_model);
                
        ecs.system("render_end")
            .kind(flecs::OnUpdate)
            .run(end_render);
    }
}