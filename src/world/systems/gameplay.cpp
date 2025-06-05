#include <flecs.h>
#include <raylib.h>
#include <raymath.h>
#include "world/components/gameplay.h"
#include "world/components/render.h"
#include "world/world.h"

namespace gameplay_systems {
    void move_target(flecs::iter &iter) {
        bool should_move = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        auto *cam = should_move ? iter.world().get_mut<WorldCamera>() : nullptr;
        
        while (iter.next()) {
            if (!should_move || cam == nullptr) {
                continue;
            }
            
            auto position = iter.field<MoveTo>(0);
            auto mouseRay = GetMouseRay(GetMousePosition(), cam->camera);

            for (auto i : iter) {
                float target = -mouseRay.position.y / mouseRay.direction.y;
                if (target > 0.0F) {
                    position[i].target = Vector3Add(mouseRay.position, Vector3Scale(mouseRay.direction, target));
                }
            }
        }
    }

    void move_to(flecs::entity entity, MoveTo &move_to, WorldTransform &transform, Animation &animation) {
        Vector3 direction = Vector3Subtract(move_to.target, transform.pos);
        Vector3 forward = Vector3Normalize(direction);

        if (Vector3Length(direction) < move_to.speed) {
            transform.pos = move_to.target;
            animation.index = 1;
        } else {
            transform.pos = Vector3Add(transform.pos, Vector3Scale(forward, move_to.speed));
            transform.yaw = atan2f(forward.x, forward.z) * (180.0F / PI);
            animation.index = 2;
            animation.speed = 480;
        }
    }

    void register_systems(World &world) {
        world.ecs.system<MoveTo>("move_target")
            .kind(world.fixed_phase)
            .run(move_target);

        world.ecs.system<MoveTo, WorldTransform, Animation>("move_to")
            .kind(world.fixed_phase)
            .each(move_to);
    }
}
