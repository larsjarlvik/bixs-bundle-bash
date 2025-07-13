#include "FastNoiseLite.h"
#include "game.h"
#include <raylib.h>
#include <raymath.h>
#include "terrain.h"
#include "world/components/render.h"
#include <vector>

namespace terrain {
    void generate_water(const World &world) {
        const auto depth_texture = LoadTextureFromImage({
            .data = elevation.data(),
            .width = DETAILED_SIZE,
            .height = DETAILED_SIZE,
            .mipmaps = 1,
            .format = PIXELFORMAT_UNCOMPRESSED_R32
        });
        SetTextureFilter(depth_texture, TEXTURE_FILTER_BILINEAR);


        const auto water_texture = LoadTextureFromImage(LoadImage(ASSET_PATH("textures/water-normal.jpg")));

        SetTextureFilter(water_texture, TEXTURE_FILTER_BILINEAR);
        SetTextureWrap(water_texture, TEXTURE_WRAP_REPEAT);

        const auto water_shader { world.ecs.get<WaterShader>() };
        const auto water_model { LoadModelFromMesh(GenMeshPlane(WORLD_SIZE, WORLD_SIZE, 1, 1)) };

        water_model.materials[0].shader = water_shader->shader;
        water_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = depth_texture;
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