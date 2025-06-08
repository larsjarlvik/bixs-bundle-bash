#include "world/components/interpolation.h"

#include "raymath.h"
#include "world/world.h"
#include "world/components/render.h"

namespace interpolation_systems {
    void store_previous(const flecs::entity entity, const WorldTransform &transform) {
        auto& state = entity.ensure<InterpolationState>();
        state.prev_pos = transform.pos;
        state.prev_yaw = transform.yaw;
    }

    void set_render_state(const flecs::iter &iter, size_t, const WorldTransform &transform, InterpolationState &state) {
        const float alpha = iter.delta_time();

        state.render_pos = Vector3Lerp(state.prev_pos, transform.pos, alpha);
        state.render_yaw = Lerp(state.prev_yaw, transform.yaw, alpha);
    }

    void register_systems(const World &world) {
        world.ecs.system<WorldTransform>("store_previous")
            .kind(world.fixed_phase)
            .each(store_previous);

        world.ecs.system<WorldTransform, InterpolationState>("set_render_state")
            .kind(world.render_phase)
            .each(set_render_state);
    }
}