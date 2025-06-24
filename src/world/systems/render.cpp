#include <cstddef>
#include <raylib.h>
#include <flecs.h>
#include <raymath.h>
#include "world/components/render.h"
#include <iostream>

#include "rlgl.h"
#include "world/world.h"
#include "world/components/interpolation.h"
#include "world/components/particle.h"

constexpr float animation_speed { 240.0F };

namespace render_systems {
    void register_systems(const World &world) {
        // Follow an object with the camera
        const auto camera_follow { [&ecs = world.ecs](flecs::entity, const InterpolationState& state) {
            auto *cam { ecs.get_mut<WorldCamera>() };
            cam->camera.target = state.render_pos;
        }};

        // Update camera position based on camera target and distance
        const auto update_camera { [&ecs = world.ecs](const flecs::entity entity) {
            auto *cam { ecs.get_mut<WorldCamera>() };
            const auto *world_shader { entity.world().get<WorldShader>() };
            const auto *particle_shader { entity.world().get<WorldShader>() };

            SetShaderValue(world_shader->shader, world_shader->loc_view_pos, &cam->camera.position, SHADER_UNIFORM_VEC3);
            SetShaderValue(particle_shader->shader, particle_shader->loc_view_pos, &cam->camera.position, SHADER_UNIFORM_VEC3);
            cam->camera.position = Vector3Add(
                cam->camera.target,
                {cam->distance, cam->distance * 1.5F, cam->distance}
            );
        }};

        // Initiate rendering in raylib
        const auto begin_render { [&ecs = world.ecs](flecs::iter) {
            const auto *cam { ecs.get<WorldCamera>() };
            BeginMode3D(cam->camera);
        }};

        // Setup lighting properties in shader
        const auto setup_lighting { [](flecs::entity, const WorldShader &shader) {
            constexpr Vector3 light_dir { -0.5F, -1.0F, -0.5F };
            constexpr Vector3 light_color { 0.4F, 0.4F, 0.4F };

            SetShaderValue(shader.shader, shader.loc_light_dir, &light_dir, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader.shader, shader.loc_light_color, &light_color, SHADER_UNIFORM_VEC3);
        }};

        // Animate models
        const auto animate_model { [](const flecs::iter& iter, size_t, WorldModel &model, Animation &anim) {
            const auto animation { model.animations[anim.run_once.value_or(anim.name)] };
            anim.frame_time += iter.delta_time();

            const float animation_duration { static_cast<float>(model.animations[anim.name].frameCount) / animation_speed };

            while (anim.frame_time >= animation_duration) {
                anim.frame_time -= animation_duration;
                anim.run_once.reset();
            }

            const auto current_frame { static_cast<int>(anim.frame_time * animation_speed) };
            UpdateModelAnimation(model.model, animation, current_frame);
        }};

        // Render models
        const auto render_model { [](const flecs::entity entity, WorldModel &model, const InterpolationState &state) {
            const auto *shader { entity.world().get<WorldShader>() };

            const auto shader_bool { static_cast<int>(model.textured) };
            SetShaderValue(shader->shader, shader->loc_use_texture, &shader_bool, SHADER_UNIFORM_INT);

            for (int i { 0 }; i < model.model.materialCount; i++) {
                model.model.materials[i].shader = shader->shader;
            }

            const auto mat_rotation { QuaternionToMatrix(state.render_rot) };
            const auto mat_translation { MatrixTranslate(state.render_pos.x, state.render_pos.y, state.render_pos.z) };
            const auto mat_transform { MatrixMultiply(mat_rotation, mat_translation) };

            model.model.transform = mat_transform;

            DrawModel(model.model, {}, 1.0F, WHITE);
        }};

        // Render particles
        const auto render_particle = [](flecs::iter& iter) {
            const auto* shader = iter.world().get<WorldShader>();

            BeginShaderMode(shader->shader);

            const auto query { iter.world().query<Particle, InterpolationState>() };
            query.each([](const Particle& particle, const InterpolationState& state) {
                const auto particle_size { 0.1F * particle.lifetime };
                // Convert euler to rotation matrix (more efficient for complex rotations)
                Matrix rotation_matrix = MatrixRotateXYZ({
                    state.render_rot.x, // Convert to radians if needed
                    state.render_rot.y,
                    state.render_rot.z,
                });

                Matrix translation = MatrixTranslate(state.render_pos.x, state.render_pos.y, state.render_pos.z);
                Matrix scale = MatrixScale(particle_size, particle_size, particle_size);

                // Combine: Scale -> Rotate -> Translate
                Matrix transform = MatrixMultiply(scale, rotation_matrix);
                transform = MatrixMultiply(transform, translation);

                rlPushMatrix();
                rlMultMatrixf(MatrixToFloat(transform));
                DrawCube({0, 0, 0}, 1.0f, 1.0f, 1.0f, particle.color);
                rlPopMatrix();
            });

            EndShaderMode();
        };

        // End raylib render
        const auto end_render { [](flecs::iter) {
            EndMode3D();
        }};

        world.ecs.system<InterpolationState>("camera_follow")
            .kind(world.render_phase)
            .with<CameraFollow>()
            .each(camera_follow);

        world.ecs.system("update_camera")
            .kind(world.render_phase)
            .with<WorldCamera>()
            .each(update_camera);

        world.ecs.system("begin_render")
            .kind(world.render_phase)
            .run(begin_render);

        world.ecs.system<WorldShader>("setup_lighting")
            .kind(world.render_phase)
            .each(setup_lighting);

        world.ecs.system<WorldModel, Animation>("animate_model")
            .kind(world.fixed_phase)
            .each(animate_model);

        world.ecs.system<WorldModel, InterpolationState>("render_model")
            .kind(world.render_phase)
            .each(render_model);

        world.ecs.system("render_particle")
            .kind(world.render_phase)
            .run(render_particle);

        world.ecs.system("end_render")
            .kind(world.render_phase)
            .run(end_render);
    }
}
