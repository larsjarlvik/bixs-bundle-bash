#include <flecs.h>
#include <raylib.h>
#include <raymath.h>
#include "world/components/camera.h"
#include "world/components/rendering.h"

namespace camera_systems {
    void update_camera(flecs::iter &iter) {
        while (iter.next()) {
            auto *cam = iter.world().get_mut<WorldCamera>();
            
            cam->camera.position = Vector3Add(
                cam->camera.target,
                {cam->distance, cam->distance, cam->distance}
            );
        }
    }

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

    void move_to(flecs::entity entity, MoveTo &move_to, Position &pos, Rotation &rot) {
        Vector3 direction = Vector3Subtract(move_to.target, pos.pos);
        Vector3 forward = Vector3Normalize(direction);

        if (Vector3Length(direction) < move_to.speed) {
            pos.pos = move_to.target;
        } else {
            pos.pos = Vector3Add(pos.pos, Vector3Scale(forward, move_to.speed));
            rot.yaw = atan2f(forward.x, forward.z) * (180.0F / PI);
        }
    }

    void register_systems(flecs::world &ecs) {
        ecs.system("update_camera")
            .kind(flecs::OnUpdate)
            .with<WorldCamera>()
            .run(update_camera);
        
        ecs.system<MoveTo>("move_target")
            .kind(flecs::OnUpdate)
            .run(move_target);

        ecs.system<MoveTo, Position, Rotation>("move_to")
            .kind(flecs::OnUpdate)
            .each(move_to);
    }
}
