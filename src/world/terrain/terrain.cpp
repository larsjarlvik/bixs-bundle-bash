#include <FastNoiseLite.h>
#include "terrain.h"

#include "game.h"

#include <vector>
#include <raylib.h>
#include <raymath.h>
#include "world/world.h"
#include "world/components/render.h"

constexpr int DETAIL { 4 };
constexpr float SCALE { 5.0f };
constexpr float FREQUENCY { 0.1f };

constexpr int DETAILED_SIZE { WORLD_SIZE * DETAIL };
constexpr auto WORLD_CENTER { static_cast<float>(WORLD_SIZE) / 2.0f };

// Fixed: Use proper dimensions for detailed terrain
static std::vector<float> heights((WORLD_SIZE * DETAIL) * (WORLD_SIZE * DETAIL));

namespace terrain {
    // Calculate the normal for a vertex in the terrain
    Vector3 calculateNormal(const std::vector<float> &heights, const int x, const int z) {

        const auto hL { (x > 0) ? heights[z * DETAILED_SIZE + (x - 1)] : heights[z * DETAILED_SIZE + x] };
        const auto hR { (x < DETAILED_SIZE - 1) ? heights[z * DETAILED_SIZE + (x + 1)] : heights[z * DETAILED_SIZE + x] };
        const auto hD { (z > 0) ? heights[(z - 1) * DETAILED_SIZE + x] : heights[z * DETAILED_SIZE + x] };
        const auto hU { (z < DETAILED_SIZE - 1) ? heights[(z + 1) * DETAILED_SIZE + x] : heights[z * DETAILED_SIZE + x] };

        const auto dx { (hR - hL) * DETAIL }; // Scale by detail level
        const auto dz { (hU - hD) * DETAIL }; // Scale by detail level

        const Vector3 tangentX { 1.0f, dx, 0.0f };
        const Vector3 tangentZ { 0.0f, dz, 1.0f };

        return Vector3Normalize(Vector3CrossProduct(tangentZ, tangentX));
    }

    // Generate the terrain
    void generate_terrain(const World &world) {
        FastNoiseLite noise;
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        noise.SetSeed(1337);
        noise.SetFrequency(FREQUENCY);

        for (int z = 0; z < DETAILED_SIZE; z++) {
            for (int x = 0; x < DETAILED_SIZE; x++) {
                const auto index { z * DETAILED_SIZE + x };
                const auto noiseValue { noise.GetNoise(static_cast<float>(x) / DETAIL, static_cast<float>(z) / DETAIL) };
                const auto distance = Vector2Distance(
                    Vector2 { WORLD_CENTER, WORLD_CENTER },
                    Vector2 { static_cast<float>(x) / static_cast<float>(DETAIL), static_cast<float>(z) / static_cast<float>(DETAIL) }
                );

                heights[index] = (noiseValue + 1.0f) * 0.5f;
                heights[index] -= distance / (static_cast<float>(WORLD_SIZE) * 0.5f);
                heights[index] *= SCALE;
            }
        }

        constexpr auto vertex_count = DETAILED_SIZE * DETAILED_SIZE;
        constexpr auto triangle_count = (DETAILED_SIZE - 1) * (DETAILED_SIZE - 1) * 2;
        Mesh mesh = {
            .vertexCount { vertex_count },
            .triangleCount { triangle_count },
            .vertices { new float[vertex_count * 3] },
            .texcoords { new float[vertex_count * 2] },
            .normals { new float[vertex_count * 3] },
            .indices { new unsigned short[triangle_count * 3] },
        };

        for (int z {0}; z < DETAILED_SIZE; ++z) {
            for (int x {0}; x < DETAILED_SIZE; ++x) {
                const auto index { z * DETAILED_SIZE + x };

                mesh.vertices[index * 3] = (static_cast<float>(x) / DETAIL) - (WORLD_SIZE / 2.0f);
                mesh.vertices[index * 3 + 1] = heights[index];
                mesh.vertices[index * 3 + 2] = (static_cast<float>(z) / DETAIL) - (WORLD_SIZE / 2.0f);

                mesh.texcoords[index * 2] = static_cast<float>(x) / (DETAILED_SIZE - 1);
                mesh.texcoords[index * 2 + 1] = static_cast<float>(z) / (DETAILED_SIZE - 1);

                const auto normal { calculateNormal(heights, x, z) };
                mesh.normals[index * 3] = normal.x;
                mesh.normals[index * 3 + 1] = normal.y;
                mesh.normals[index * 3 + 2] = normal.z;
            }
        }

        int indexCount = 0;
        for (int z = 0; z < DETAILED_SIZE - 1; z++) {
            for (int x = 0; x < DETAILED_SIZE - 1; x++) {
                const auto top_left { z * DETAILED_SIZE + x };
                const auto top_right { z * DETAILED_SIZE + (x + 1) };
                const auto bottom_left { (z + 1) * DETAILED_SIZE + x };
                const auto bottom_right { (z + 1) * DETAILED_SIZE + (x + 1) };

                mesh.indices[indexCount++] = top_left;
                mesh.indices[indexCount++] = bottom_left;
                mesh.indices[indexCount++] = top_right;

                mesh.indices[indexCount++] = top_right;
                mesh.indices[indexCount++] = bottom_left;
                mesh.indices[indexCount++] = bottom_right;
            }
        }

        UploadMesh(&mesh, false);

        auto ground_model { LoadModelFromMesh(mesh) };

        const auto ground_shader { world.ecs.get<GroundShader>() };
        const auto ground_texture { LoadTexture(ASSET_PATH("textures/grass.jpg")) };
        GenTextureMipmaps(const_cast<Texture2D*>(&ground_texture));
        SetTextureFilter(ground_texture, TEXTURE_FILTER_TRILINEAR);
        ground_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = ground_texture;
        ground_model.materials[0].shader = ground_shader->shader;
        world.ecs.set<WorldGround>({ .model { ground_model } });

        const auto water_shader { world.ecs.get<WaterShader>() };
        const auto water_model { LoadModelFromMesh(GenMeshPlane(WORLD_SIZE, WORLD_SIZE, 1, 1)) };
        water_model.materials[0].shader = water_shader->shader;
        world.ecs.set<WorldWater>({ .model { water_model } });
    }

    // Calculate height using barycentric
    float barycentric(const Vector3 v1, const Vector3 v2, const Vector3 v3, const float x, const float z) {
        const auto denominator { (v2.z - v3.z) * (v1.x - v3.x) + (v3.x - v2.x) * (v1.z - v3.z) };

        if (fabs(denominator) < 0.00001f) {
            return (v1.y + v2.y + v3.y) / 3.0f;
        }

        const auto w1 { ((v2.z - v3.z) * (x - v3.x) + (v3.x - v2.x) * (z - v3.z)) / denominator };
        const auto w2 { ((v3.z - v1.z) * (x - v3.x) + (v1.x - v3.x) * (z - v3.z)) / denominator };
        const auto w3 { 1.0f - w1 - w2 };

        return w1 * v1.y + w2 * v2.y + w3 * v3.y;
    }

    // Get height at coordinate
    float get_height(const float world_x, const float world_z) {
        const auto grid_x { (world_x + WORLD_SIZE / 2.0f) * DETAIL };
        const auto grid_z { (world_z + WORLD_SIZE / 2.0f) * DETAIL };

        // Check bounds
        if (grid_x < 0 || grid_x >= DETAILED_SIZE - 1 || grid_z < 0 || grid_z >= DETAILED_SIZE - 1) {
            return 0.0f;
        }

        // Get the grid cell containing this point
        const auto x0 { static_cast<int>(grid_x) };
        const auto z0 { static_cast<int>(grid_z) };
        const auto x1 { x0 + 1 };
        const auto z1 { z0 + 1 };

        // Get fractional part (position within the grid cell)
        const auto fx { grid_x - static_cast<float>(x0) };
        const auto fz { grid_z - static_cast<float>(z0) };

        const auto h00 { heights[z0 * DETAILED_SIZE + x0] }; // Bottom-left
        const auto h10 { heights[z0 * DETAILED_SIZE + x1] }; // Bottom-right
        const auto h01 { heights[z1 * DETAILED_SIZE + x0] }; // Top-left
        const auto h11 { heights[z1 * DETAILED_SIZE + x1] }; // Top-right

        if (fx + fz <= 1.0f) {
            return barycentric(
                Vector3{0.0f, h00, 0.0f},
                Vector3{1.0f, h10, 0.0f},
                Vector3{0.0f, h01, 1.0f},
                fx, fz
            );
        }

        return barycentric(
            Vector3{1.0f, h10, 0.0f},
            Vector3{1.0f, h11, 1.0f},
            Vector3{0.0f, h01, 1.0f},
            fx, fz
        );
    }

    // Check for terrain intersection and where
    std::optional<Vector3> ray_terrain_intersect(const Vector3& origin, const Vector3& direction) {
        float min_t = 0.0f;
        float max_t = 50.0f;

        while (max_t - min_t > 0.02f) {
            const float mid_t { (min_t + max_t) * 0.5f };
            const auto pos { origin + Vector3Scale(direction, mid_t) };

            if (pos.y <= get_height(pos.x, pos.z)) {
                max_t = mid_t;
            } else {
                min_t = mid_t;
            }
        }

        const auto final_pos { origin + Vector3Scale(direction, min_t) };
        return Vector3{ final_pos.x, get_height(final_pos.x, final_pos.z), final_pos.z };
    }
}