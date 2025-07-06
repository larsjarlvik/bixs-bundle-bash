#include <cstddef>
#include <raylib.h>
#include <flecs.h>
#include <raymath.h>
#include "world/components/render.h"

#include <algorithm>
#include <iostream>

#include "rlgl.h"
#include "world/world.h"
#include "world/components/interpolation.h"
#include "world/components/particle.h"

constexpr float animation_speed { 240.0f };

constexpr Vector3 light_dir { -0.5f, -1.0f, -0.5f };
constexpr Vector3 light_color { 0.4f, 0.4f, 0.4f };

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
            const auto *shader { entity.world().get<ModelShader>() };

            SetShaderValue(shader->shader, shader->loc_view_pos, &cam->camera.position, SHADER_UNIFORM_VEC3);
            cam->camera.position = Vector3Add(
                cam->camera.target,
                {cam->distance, cam->distance * 1.5f, cam->distance}
            );
        }};

        // Initiate rendering in raylib
        const auto begin_render { [&ecs = world.ecs](flecs::iter) {
            const auto *cam { ecs.get<WorldCamera>() };
            BeginMode3D(cam->camera);
        }};

        // Setup lighting properties in shader
        const auto setup_lighting { [](flecs::entity, const ModelShader &shader) {

            SetShaderValue(shader.shader, shader.loc_light_dir, &light_dir, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader.shader, shader.loc_light_color, &light_color, SHADER_UNIFORM_VEC3);
        }};

        // Animate models
        const auto animate_model { [](const flecs::iter& iter, size_t, WorldModel &model, Animation &anim) {
            const auto& animation = model.animations[anim.run_once.value_or(anim.name)];
            anim.frame_time += iter.delta_time();

            const auto current_frame = static_cast<int>(anim.frame_time * animation_speed);

            if (current_frame >= animation.frameCount) {
                if (anim.run_once.has_value()) {
                    anim.run_once.reset();
                    anim.frame_time = 0.0f;
                    return;
                }

                anim.frame_time = std::fmod(anim.frame_time, static_cast<float>(animation.frameCount) / animation_speed);
            }

            UpdateModelAnimation(model.model, animation, current_frame);
        }};

        // Render models
        const auto render_model { [](const flecs::iter& iter) {
            const auto* shader = iter.world().get<ModelShader>();
            BeginShaderMode(shader->shader);

            const auto query { iter.world().query<WorldModel, InterpolationState>() };

            query.each([shader](WorldModel &model, const InterpolationState &state) {
                const auto shader_bool { static_cast<int>(model.textured) };
                SetShaderValue(shader->shader, shader->loc_use_texture, &shader_bool, SHADER_UNIFORM_INT);

                for (int i { 0 }; i < model.model.materialCount; i++) {
                    model.model.materials[i].shader = shader->shader;
                }

                const auto mat_rotation { QuaternionToMatrix(state.render_rot) };
                const auto mat_translation { MatrixTranslate(state.render_pos.x, state.render_pos.y, state.render_pos.z) };
                const auto mat_transform { MatrixMultiply(mat_rotation, mat_translation) };

                model.model.transform = mat_transform;

                DrawModel(model.model, {}, 1.0f, WHITE);
            });

            EndShaderMode();
        }};

        // Render particles
        const auto render_particle = [](const flecs::iter& iter) {
            const auto* shader = iter.world().get<ModelShader>();
            BeginShaderMode(shader->shader);

            const auto query { iter.world().query<Particle, InterpolationState>() };
            query.each([](const Particle& particle, const InterpolationState& state) {
                const auto particle_size { 0.1f * particle.lifetime };
                // Convert euler to rotation matrix (more efficient for complex rotations)
                const auto rotation_matrix { MatrixRotateXYZ({
                    state.render_rot.x, // Convert to radians if needed
                    state.render_rot.y,
                    state.render_rot.z,
                })};

                const auto translation { MatrixTranslate(state.render_pos.x, state.render_pos.y, state.render_pos.z) };
                const Matrix scale { MatrixScale(particle_size, particle_size, particle_size) };

                // Combine: Scale -> Rotate -> Translate
                const auto transform { MatrixMultiply(MatrixMultiply(scale, rotation_matrix), translation) };

                rlPushMatrix();
                rlMultMatrixf(MatrixToFloat(transform));
                DrawCube({0, 0, 0}, 1.0f, 1.0f, 1.0f, particle.color);
                rlPopMatrix();
            });

            EndShaderMode();
        };

        // Render ground plane and shadows
        const auto render_ground = [](const flecs::iter& iter) {
            const auto* shader = iter.world().get<GroundShader>();
            const auto* ground = iter.world().get<WorldGround>();
            const auto *cam { iter.world().get<WorldCamera>() };

            std::vector<Vector3> positions;
            std::vector<float> radii;
            std::vector<float> intensities;

            const auto query { iter.world().query<ShadowCaster, InterpolationState>() };
            query.each([&positions, &radii, &intensities](const ShadowCaster& caster, const InterpolationState& state) {
                const auto t { -state.render_pos.y / light_dir.y };
                positions.push_back((Vector3){
                    .x = state.render_pos.x + light_dir.x * t,
                    .y = 0.0f,
                    .z = state.render_pos.z + light_dir.z * t
                });

                radii.push_back(caster.radius * (1.0f + state.render_pos.y));
                intensities.push_back(1.0f / (1.0f + state.render_pos.y * 6.0f));
            });

            // Prioritize shadows closer to the camera target
            std::sort(positions.begin(), positions.end(), [&cam](const Vector3 &a, const Vector3 &b) {
                return Vector3Distance(cam->camera.target, a) < Vector3Distance(cam->camera.target, b);
            });

            const auto shadow_count = static_cast<int>(std::min(positions.size(), static_cast<size_t>(64)));

            BeginShaderMode(shader->shader);
            SetShaderValue(shader->shader, shader->loc_ground_size, &ground->size, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader->shader, shader->loc_shadow_count, &shadow_count, SHADER_UNIFORM_INT);
            SetShaderValueV(shader->shader, shader->loc_shadow_positions, positions.data(), SHADER_UNIFORM_VEC3, shadow_count);
            SetShaderValueV(shader->shader, shader->loc_shadow_radii,radii.data(), SHADER_UNIFORM_FLOAT, shadow_count);
            SetShaderValueV(shader->shader, shader->loc_shadow_itensities, intensities.data(), SHADER_UNIFORM_FLOAT, shadow_count);

            DrawModel(ground->model, Vector3Zero(), 1.0f, WHITE);
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

        world.ecs.system<ModelShader>("setup_lighting")
            .kind(world.render_phase)
            .each(setup_lighting);

        world.ecs.system<WorldModel, Animation>("animate_model")
            .kind(world.fixed_phase)
            .each(animate_model);

        world.ecs.system("render_model")
            .kind(world.render_phase)
            .run(render_model);

        world.ecs.system("render_particle")
            .kind(world.render_phase)
            .run(render_particle);

        world.ecs.system("render_ground")
            .kind(world.render_phase)
            .run(render_ground);

        world.ecs.system("end_render")
            .kind(world.render_phase)
            .run(end_render);
    }
}
