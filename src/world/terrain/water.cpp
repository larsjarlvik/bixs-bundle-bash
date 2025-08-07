#include "FastNoiseLite.h"
#include "game.h"
#include <raylib.h>
#include <raymath.h>
#include "terrain.h"
#include "world/components/render.h"
#include <vector>

namespace terrain {
    void generate_water(const World &world) {
        const auto water_texture = LoadTextureFromImage(LoadImage(ASSET_PATH("textures/water-normal.jpg")));
        SetTextureFilter(water_texture, TEXTURE_FILTER_BILINEAR);
        SetTextureWrap(water_texture, TEXTURE_WRAP_REPEAT);

        const auto water_shader { world.ecs.get<WaterShader>() };

        const auto mesh = GenMeshPlane(WORLD_SIZE, WORLD_SIZE, DETAILED_SIZE - 1, DETAILED_SIZE - 1);
        const auto vertex_count = mesh.vertexCount;
        for (int i = 0; i < vertex_count; ++i) {
            mesh.vertices[i * 3 + 1] = elevation[i];
        }

        UpdateMeshBuffer(mesh, 0, mesh.vertices, static_cast<int>(vertex_count * 3 * sizeof(float)), 0);

        const auto water_model { LoadModelFromMesh(mesh) };

        water_model.materials[0].shader = water_shader->shader;
        water_model.materials[0].maps[MATERIAL_MAP_NORMAL].texture = water_texture;
        world.ecs.set<WorldWater>({ .model { water_model } });
    }

    std::optional<Vector3> find_closest_shallow_point(const Vector3& target, const Vector3& source, float depth) {
        constexpr auto step_size = 0.1f;
        const auto dir = Vector3Subtract(target, source);
        const auto dist = Vector3Length(dir);
        const auto step = Vector3Scale(Vector3Normalize(dir), step_size);

        std::optional<Vector3> furthest_shallow_point;

        for (float t = 0; t <= dist; t += step_size) {
            const auto pos = Vector3Add(source, Vector3Scale(step, t / step_size));

            if (get_height(pos.x, pos.z) < -depth) {
                break;
            }

            furthest_shallow_point = Vector3{ pos.x, get_height(pos.x, pos.z), pos.z };
        }

        return furthest_shallow_point;
    }
}