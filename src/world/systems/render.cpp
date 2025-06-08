#include <cstddef>
#include <raylib.h>
#include <flecs.h>
#include <raymath.h>
#include "world/components/render.h"
#include "world/world.h"
#include "world/components/interpolation.h"

namespace render_systems {
    void update_camera(flecs::iter &iter) {
        while (iter.next()) {
            auto *cam = iter.world().get_mut<WorldCamera>();
            
            cam->camera.position = Vector3Add(
                cam->camera.target,
                {cam->distance, cam->distance, cam->distance}
            );
        }
    }

    void begin_render(flecs::iter &iter) {
        while (iter.next()) {
            const auto *cam = iter.world().get<WorldCamera>();
            BeginMode3D(cam->camera);
        }
    }

    void setup_lighting(flecs::entity, const WorldShader &shader) {
        constexpr Vector3 light_dir = { -0.5F, -1.0F, -0.5F };
        constexpr Vector3 light_color = { 0.4F, 0.4F, 0.4F };

        SetShaderValue(shader.shader, shader.loc_light_dir, &light_dir, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader.shader, shader.loc_light_color, &light_color, SHADER_UNIFORM_VEC3);
    }

    void animate_model(const flecs::iter& iter, size_t, WorldModel &model, Animation &anim) {
        const auto animation = model.animations[anim.name];
        anim.frame_time += iter.delta_time();

        const float animation_duration = static_cast<float>(model.animations[anim.name].frameCount) / 60.0F;
        
        while (anim.frame_time >= animation_duration) {
            anim.frame_time -= animation_duration;
        }

        const auto current_frame = static_cast<int>(anim.frame_time * 240.0F);
        UpdateModelAnimation(model.model, animation, current_frame);
    }

    void render_model(const flecs::entity entity, WorldModel &model, const WorldShader &shader, const InterpolationState &state) {
        const auto *cam = entity.world().get<WorldCamera>();
        if (cam == nullptr) {
            return;
        }
        
        const auto shader_bool = static_cast<int>(model.textured);
        SetShaderValue(shader.shader, shader.loc_use_texture, &shader_bool, SHADER_UNIFORM_INT);
        SetShaderValue(shader.shader, shader.loc_view_pos, &cam->camera.position, SHADER_UNIFORM_VEC3);


        for (int i = 0; i < model.model.materialCount; i++) {
            model.model.materials[i].shader = shader.shader;
        }
        
        const auto mat_rotation = QuaternionToMatrix(QuaternionFromAxisAngle((Vector3){ 0, 1, 0 }, DEG2RAD * state.render_yaw));
        const auto mat_translation = MatrixTranslate(state.render_pos.x, state.render_pos.y, state.render_pos.z);
        const auto mat_transform = MatrixMultiply(mat_rotation, mat_translation);
        
        model.model.transform = mat_transform;

        DrawModel(model.model, {}, 1.0F, WHITE);
    }

    void end_render(flecs::iter) {
        EndMode3D();
    }

    void register_systems(const World &world) {
        world.ecs.system("update_camera")
            .kind(world.render_phase)
            .with<WorldCamera>()
            .run(update_camera);
        
        world.ecs.system("begin_render")
            .kind(world.render_phase)
            .with<WorldCamera>()
            .run(begin_render);

        world.ecs.system<WorldShader>("setup_lighting")
            .kind(world.render_phase)
            .each(setup_lighting);

        world.ecs.system<WorldModel, Animation>("animate_model") 
            .kind(world.fixed_phase)
            .each(animate_model);
            
        world.ecs.system<WorldModel, WorldShader, InterpolationState>("render_model")
            .kind(world.render_phase)
            .each(render_model);
                
        world.ecs.system("end_render")
            .kind(world.render_phase)
            .run(end_render);
    }
}
