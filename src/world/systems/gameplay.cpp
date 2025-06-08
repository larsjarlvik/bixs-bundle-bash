#include <flecs.h>
#include <raylib.h>
#include <raymath.h>
#include "world/components/gameplay.h"
#include "world/components/render.h"
#include "world/world.h"

namespace gameplay_systems {
    void move_target(flecs::iter &iter) {
        const auto should_move = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        const auto *cam = should_move ? iter.world().get_mut<WorldCamera>() : nullptr;
        
        while (iter.next()) {
            if (!should_move || cam == nullptr) {
                continue;
            }
            
            auto move_to = iter.field<MoveTo>(0);
            const auto [position, direction] = GetMouseRay(GetMousePosition(), cam->camera);

            for (const auto i : iter) {
                if (const auto target = -position.y / direction.y; target > 0.0F) {
                    move_to[i].target = Vector3Add(position, Vector3Scale(direction, target));
                }
            }
        }
    }

    void move_to(const MoveTo &move_to, WorldTransform &transform, Animation &animation) {
        const auto direction = Vector3Subtract(move_to.target, transform.pos);
        const auto forward = Vector3Normalize(direction);

        if (Vector3Length(direction) < move_to.speed) {
            transform.pos = move_to.target;
            animation.name = "Idle";
        } else {
            transform.pos = Vector3Add(transform.pos, Vector3Scale(forward, move_to.speed));
            transform.yaw = atan2f(forward.x, forward.z) * (180.0F / PI);
            animation.name = "Run";
        }
    }

    void register_systems(const World &world) {
        world.ecs.system<MoveTo>("move_target")
            .kind(world.fixed_phase)
            .run(move_target);

        world.ecs.system<MoveTo, WorldTransform, Animation>("move_to")
            .kind(world.fixed_phase)
            .each(move_to);
    }
}
