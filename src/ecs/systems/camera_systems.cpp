#include <flecs.h>
#include <raylib.h>
#include <raymath.h>
#include "../components/camera.h"

void update_camera(flecs::iter &iter) {
    auto *cam = iter.world().get_mut<WorldCamera>();
    if (cam == nullptr) {
        return;
    }

    cam->camera.position = Vector3Add(
        cam->camera.target,
        {cam->distance, cam->distance, cam->distance}
    );
}
