#include "world/components/particle.h"
#include "world/components/render.h"
#include "particle.h"

#include <cmath>

#include "world/world.h"

#include <iostream>

#include "raymath.h"
#include "util.h"

namespace particle_systems {
    void register_systems(const World &world) {
        const auto particle_system { [&ecs = world.ecs](const flecs::entity entity, Particle &particle, WorldTransform &transform) {
            particle.lifetime -= FIXED_DT;

            if (particle.lifetime < 0.0) {
                ecs.defer([&]() {
                    entity.destruct();
                });
            } else {
                particle.velocity.y -= 9.8f * FIXED_DT;
                transform.pos = Vector3Add(transform.pos, particle.velocity * FIXED_DT);
                transform.rot = Vector3Add(transform.rot, particle.rot_velocity * FIXED_DT);
            }
        }};

        const auto explosion_system { [&ecs = world.ecs](const flecs::entity entity, const Explosion &explosion, const WorldTransform &transform) {
            for (int i { 0 }; i < explosion.particles; ++i) {

                const auto theta { util::GetRandomFloat(0.0f, 360.0f) * DEG2RAD };
                const auto phi { util::GetRandomFloat(0.0f, 180.0f) * DEG2RAD };
                const auto speed { util::GetRandomFloat(0.5f, 1.0f) };
                const auto rot_speed { util::GetRandomFloat(1.0f, 5.0f) };


                ecs.entity()
                    .set<WorldTransform>(transform)
                    .set<Particle>({
                        .lifetime { util::GetRandomFloat(0.5f, 1.0f) },
                        .variation { util::GetRandomFloat(-5.0f, 5.0f) },
                        .color { explosion.colors[util::GetRandomInt(0, static_cast<int>(explosion.colors.size()) - 1)] },
                        .velocity { (Vector3){
                            std::cosf(theta) * sinf(phi) * speed,
                            std::cosf(phi) * speed + 3.0f,
                            std::sinf(theta) * sinf(phi) * speed
                        }},
                        .rot_velocity {(Vector3){
                            util::GetRandomFloat(0.0f, 360.0f) * rot_speed,
                            util::GetRandomFloat(0.0f, 360.0f) * rot_speed,
                            util::GetRandomFloat(0.0f, 360.0f) * rot_speed,
                        }}
                    });
            }

            ecs.defer([&]() {
                entity.destruct();
            });
        }};

        world.ecs.system<Particle, WorldTransform>("particle_system")
            .kind(world.fixed_phase)
            .each(particle_system);

        world.ecs.system<Explosion, WorldTransform>("explosion_system")
            .kind(world.fixed_phase)
            .each(explosion_system);

    }
}
