#pragma once
#include <optional>
#include "raylib.h"
#include "world/world.h"

namespace terrain {
    void generate_terrain(const World &world);
    float get_height(float world_x, float world_z);
    std::optional<Vector3> ray_terrain_intersect(const Vector3& origin, const Vector3& direction);
}
