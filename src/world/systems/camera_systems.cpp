#include <flecs.h>
#include <raylib.h>
#include <raymath.h>
#include "world/components/camera.h"
#include "world/systems/camera_systems.h"
#include "world/components/rendering.h"

void update_camera(flecs::iter &iter) {
    while (iter.next()) {
        auto *cam = iter.world().get_mut<WorldCamera>();
        
        cam->camera.position = Vector3Add(
            cam->camera.target,
            {cam->distance, cam->distance, cam->distance}
        );
    }
}

void mouse_target(flecs::iter &iter) {
    bool should_process = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    auto *cam = should_process ? iter.world().get_mut<WorldCamera>() : nullptr;
    
    while (iter.next()) {
        if (!should_process || cam == nullptr) {
            continue;
        }
        
        auto position = iter.field<WorldPos>(0);
        auto mouseRay = GetMouseRay(GetMousePosition(), cam->camera);

        for (auto i : iter) {
            float target = -mouseRay.position.y / mouseRay.direction.y;
            if (target > 0.0F) {
                position[i].pos = Vector3Add(mouseRay.position, Vector3Scale(mouseRay.direction, target));
            }
        }
    }
}