#pragma once
#include <optional>
#include "raylib.h"

#include "world/world.h"
#include <vector>
#include <micropather.h>

constexpr int DETAIL { 2 };
constexpr int WORLD_SIZE = 64;

constexpr int DETAILED_SIZE { WORLD_SIZE * DETAIL };
constexpr auto WORLD_CENTER { static_cast<float>(WORLD_SIZE) / 2.0f };

constexpr int GRID_DETAIL = 4;
constexpr int GRID_SIZE = WORLD_SIZE * GRID_DETAIL;

namespace terrain {
    extern std::vector<float> elevation;

    void generate_ground(const World &world);
    void generate_water(const World &world);

    float get_height(float world_x, float world_z);

    std::optional<Vector3> ray_ground_intersect(const Vector3& origin, const Vector3& direction);
    std::optional<Vector3> find_closest_shallow_point(const Vector3& target, const Vector3& source, float depth = 0.5f);

    bool is_walkable(int x, int y);
    void block_tile(int x, int y);
    void block_object(const Vector3& world_pos, float radius);
    void find_path(Vector3 start, Vector3 end, std::vector<Vector3>& path);
    float world_to_grid(float world_coord);
    float grid_to_world(float grid_coord);

}
