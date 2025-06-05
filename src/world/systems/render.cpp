#include <cstddef>
#include <raylib.h>
#include <flecs.h>
#include <raymath.h>
#include "world/components/render.h"
#include "world/world.h"

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

    void setup_lighting(flecs::entity entity, WorldShader &shader) {
        Vector3 light_dir = { -0.5F, -1.0F, -0.5F };
        Vector3 light_color = { 0.4F, 0.4F, 0.4F };

        SetShaderValue(shader.shader, shader.loc_light_dir, &light_dir, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader.shader, shader.loc_light_color, &light_color, SHADER_UNIFORM_VEC3);
    }

    void animate_model(flecs::iter& iter, size_t t, WorldModel &model, Animation &anim) {
        if (model.animations == nullptr || model.animations[anim.index].frameCount <= 0) {
            return;
        }
        
        anim.frame_time += iter.delta_time() * anim.speed;
        
        int frame_count = model.animations[anim.index].frameCount;
        while (anim.frame_time >= frame_count) {
            anim.frame_time -= frame_count;
        }
        
        UpdateModelAnimation(model.model, model.animations[anim.index], (int)anim.frame_time);
    }

    void render_model(flecs::entity entity, WorldModel &model, WorldShader &shader, WorldTransform &transform) {
        const auto *cam = entity.world().get<WorldCamera>();
        if (cam == nullptr) {
            return;
        }
        
        auto shader_bool = static_cast<int>(model.textured);
        SetShaderValue(shader.shader, shader.loc_use_texture, &shader_bool, SHADER_UNIFORM_INT);
        SetShaderValue(shader.shader, shader.loc_view_pos, &cam->camera.position, SHADER_UNIFORM_VEC3);


        for (int i = 0; i < model.model.materialCount; i++) {
            model.model.materials[i].shader = shader.shader;
        }
        
        auto mat_rotation = QuaternionToMatrix(QuaternionFromAxisAngle((Vector3){ 0, 1, 0 }, DEG2RAD * transform.yaw));
        auto mat_translation = MatrixTranslate(transform.pos.x, transform.pos.y, transform.pos.z);
        auto mat_transform = MatrixMultiply(mat_rotation, mat_translation);
        
        model.model.transform = mat_transform;

        DrawModel(model.model, {0}, 1.0F, WHITE);
    }

    void end_render(flecs::iter &iter) {
        EndMode3D();
    }

    void register_systems(World &world) {
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
            
        world.ecs.system<WorldModel, WorldShader, WorldTransform>("render_model")
            .kind(world.render_phase)
            .each(render_model);
                
        world.ecs.system("render_end")
            .kind(world.render_phase)
            .run(end_render);
    }
}