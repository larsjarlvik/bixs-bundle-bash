#include "terrain.h"
#include <raylib.h>
#include <vector>
#include <cmath>
#include "micropather.h"


namespace terrain {
    std::vector walkable(GRID_SIZE * GRID_SIZE, true);

    inline bool is_in_bounds(const int x, const int y) {
        return x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE;
    }

    inline int coords_to_index(const int x, const int y) {
        return y * GRID_SIZE + x;
    }

    inline std::pair<int, int> index_to_coords(const unsigned int index) {
        return {index % GRID_SIZE, index / GRID_SIZE};
    }

    bool is_walkable(const int x, const int y) {
        return is_in_bounds(x, y) && walkable[coords_to_index(x, y)];
    }

    float world_to_grid(const float world_coord) {
        return (world_coord + WORLD_CENTER) * GRID_DETAIL;
    }

    float grid_to_world(const float grid_coord) {
        return (grid_coord / GRID_DETAIL) - WORLD_CENTER;
    }

    std::pair<int, int> world_to_grid_coords(const Vector3& world_pos) {
        const auto gx { static_cast<int>(std::round(world_to_grid(world_pos.x))) };
        const auto gz { static_cast<int>(std::round(world_to_grid(world_pos.z))) };
        return { gx, gz };
    }

    void block_tile(const int x, const int y) {
        if (is_in_bounds(x, y)) {
            walkable[coords_to_index(x, y)] = false;
        }
    }
    void block_object(const Vector3& world_pos, const float radius) {
        const auto [grid_x, grid_z] { world_to_grid_coords(world_pos) };
        const auto grid_radius { static_cast<int>(std::ceil(radius * GRID_DETAIL)) };
        const int r2 = grid_radius * grid_radius;

        for (auto dy { -grid_radius }; dy <= grid_radius; ++dy) {
            for (auto dx { -grid_radius }; dx <= grid_radius; ++dx) {
                if (dx * dx + dy * dy <= r2) {
                    block_tile(grid_x + dx, grid_z + dy);
                }
            }
        }
    }

    bool is_position_walkable(const Vector3& world_pos) {
        const auto [gx, gz] { world_to_grid_coords(world_pos) };
        return is_walkable(gx, gz);
    }

    class GridGraph final : public micropather::Graph {
        static constexpr std::pair<int, int> directions[8] = {
            {1, 0}, {-1, 0}, {0, 1}, {0, -1},  // Cardinal
            {1, 1}, {1, -1}, {-1, 1}, {-1, -1} // Diagonal
        };

    public:
        float LeastCostEstimate(void* state1, void* state2) override {
            auto [x1, y1] { index_to_coords(reinterpret_cast<uintptr_t>(state1)) };
            auto [x2, y2] { index_to_coords(reinterpret_cast<uintptr_t>(state2)) };

            const auto dx { static_cast<float>(x1 - x2) };
            const auto dy { static_cast<float>(y1 - y2) };
            return std::sqrt(dx * dx + dy * dy);
        }

        void AdjacentCost(void* state, micropather::MPVector<micropather::StateCost>* adjacent) override {
            auto [x, y] { index_to_coords(reinterpret_cast<uintptr_t>(state)) };

            for (const auto& [dx, dy] : directions) {
                const auto nx { x + dx };
                const auto ny { y + dy };

                if (is_walkable(nx, ny)) {
                    const auto next_state = reinterpret_cast<void*>(coords_to_index(nx, ny));
                    const auto cost = (dx != 0 && dy != 0) ? 1.414f : 1.0f;
                    adjacent->push_back({next_state, cost});
                }
            }
        }

        void PrintStateInfo(void*) override {}
    };

    // Static pathfinding objects
    static GridGraph graph;
    static micropather::MicroPather pather(&graph, 10000);

    // Helper function to find nearest walkable point to target
    std::pair<int, int> find_nearest_walkable(const int target_x, const int target_z, const int max_radius = 50) {
        for (auto radius { 1 }; radius <= max_radius; ++radius) {
            for (auto dz { -radius }; dz <= radius; ++dz) {
                for (auto dx { -radius }; dx <= radius; ++dx) {
                    if (std::abs(dx) == radius || std::abs(dz) == radius) {
                        const auto x { target_x + dx };
                        const auto z { target_z + dz };

                        if (is_walkable(x, z)) {
                            return {x, z};
                        }
                    }
                }
            }
        }

        return {-1, -1}; // Nothing better found
    }

    bool has_line_of_sight(const Vector3& from, const Vector3& to) {
        const auto [from_x, from_z] { world_to_grid_coords(from) };
        const auto [to_x, to_z] { world_to_grid_coords(to) };

        int dx = std::abs(to_x - from_x), dy = std::abs(to_z - from_z);
        int sx = from_x < to_x ? 1 : -1, sy = from_z < to_z ? 1 : -1;
        int err = dx - dy, x = from_x, y = from_z;

        while (x != to_x || y != to_z) {
            if (!is_walkable(x, y)) return false;

            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x += sx; }
            if (e2 < dx) { err += dx; y += sy; }
        }

        return is_walkable(to_x, to_z);
    }

    void smooth_path(std::vector<Vector3>& path) {
        if (path.size() < 3) return;

        std::vector<Vector3> smoothed;
        smoothed.push_back(path[0]);

        size_t current = 0;
        while (current < path.size() - 1) {
            size_t farthest = current + 1;

            for (size_t i = current + 2; i < path.size(); ++i) {
                if (has_line_of_sight(path[current], path[i])) {
                    farthest = i;
                }
            }

            smoothed.push_back(path[farthest]);
            current = farthest;
        }

        path = std::move(smoothed);
    }

    void find_path(const Vector3 start, const Vector3 end, std::vector<Vector3>& path) {
        const auto start_x { static_cast<int>(std::round(world_to_grid(start.x))) };
        const auto start_z { static_cast<int>(std::round(world_to_grid(start.z))) };
        auto end_x { static_cast<int>(std::round(world_to_grid(end.x))) };
        auto end_z { static_cast<int>(std::round(world_to_grid(end.z))) };

        if (!is_in_bounds(start_x, start_z) || !is_in_bounds(end_x, end_z)) {
            return;
        }

        if (!is_walkable(end_x, end_z)) {
            auto [new_x, new_z] = find_nearest_walkable(end_x, end_z);
            if (new_x == -1) return;

            end_x = new_x;
            end_z = new_z;
        }

        const auto start_state { reinterpret_cast<void*>(coords_to_index(start_x, start_z)) };
        const auto end_state { reinterpret_cast<void*>(coords_to_index(end_x, end_z)) };

        // Solve path
        micropather::MPVector<void*> solution;
        float total_cost;

        const auto result { pather.Solve(start_state, end_state, &solution, &total_cost) };
        if (result != micropather::MicroPather::SOLVED) {
            return;
        }

        path.reserve(solution.size());

        for (size_t i { 0 }; i < solution.size(); ++i) {
            auto [x, z] = index_to_coords(reinterpret_cast<uintptr_t>(solution[i]));

            Vector3 world_pos = {
                grid_to_world(static_cast<float>(x)),
                0.0f,
                grid_to_world(static_cast<float>(z))
            };

            world_pos.y = get_height(world_pos.x, world_pos.z);
            path.push_back(world_pos);
        }

        smooth_path(path);
    }
}