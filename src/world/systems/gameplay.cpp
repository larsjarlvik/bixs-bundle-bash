#include <flecs.h>
#include <raylib.h>
#include <raymath.h>
#include <iostream>

#include "world/components/gameplay.h"
#include "world/components/render.h"
#include "world/components/particle.h"
#include "world/world.h"

namespace gameplay_systems {
    void register_systems(const World &world) {
        // Sets the MoveTo component to where the player clicks
        const auto move_target_system { [](flecs::iter &iter) {
            const auto should_move { IsMouseButtonDown(MOUSE_LEFT_BUTTON) };
            const auto *cam { should_move ? iter.world().get_mut<WorldCamera>() : nullptr };

            while (iter.next()) {
                if (should_move && cam != nullptr) {
                    auto move_to { iter.field<MoveTo>(0) };
                    const auto [position, direction] { GetMouseRay(GetMousePosition(), cam->camera) };

                    for (const auto i : iter) {
                        if (const auto target { -position.y / direction.y }; target > 0.0F) {
                            move_to[i].target = Vector3Add(position, Vector3Scale(direction, target));
                        }
                    }
                }
            }
        }};

        // Moves an animated entity with a transform towards MoveTo
        const auto move_to_system { [](const MoveTo &move_to, WorldTransform &transform, Animation &animation) {
            const auto direction { Vector3Subtract(move_to.target, transform.pos) };
            const auto forward { Vector3Normalize(direction) };

            if (Vector3Length(direction) < move_to.speed) {
                transform.pos = move_to.target;
                animation.name = "Idle";
            } else {
                transform.pos = Vector3Add(transform.pos, Vector3Scale(forward, move_to.speed));
                transform.rot.y = atan2f(forward.x, forward.z) * (180.0F / PI);
                animation.name = "Run";
            }
        }};

        // Make an entity spin
        const auto spin_system { [](const Spin &spin, WorldTransform &transform) {
            transform.rot.y += spin.speed;
        }};

        // Makes an entity bounce
        const auto bounce_system { [](Bounce &bounce, WorldTransform &transform) {
            transform.pos.y = bounce.center_y +  sinf(bounce.elapsed) * bounce.height;
            bounce.elapsed += bounce.speed;
        }};

        const auto eat_system { [&ecs = world.ecs](const Consumer &consumer, const WorldTransform &transform, Animation &animation) {
            ecs.each([&](const flecs::entity consumable_entity, const Consumable &consumable, const WorldTransform &consumable_transform) {
                const float distance = Vector2Distance(
                    { .x = transform.pos.x, .y = transform.pos.z },
                    { .x = consumable_transform.pos.x, .y = consumable_transform.pos.z
                });

                if (distance <= consumer.range) {
                    consumable_entity.destruct();
                    ecs.entity()
                        .set<WorldTransform>(consumable_transform)
                        .set<Explosion>({
                            .colors = consumable.colors,
                            .particles = consumable.particles,
                        });

                    if (!animation.run_once.has_value()) {
                        animation.run_once = "Eat";
                        animation.frame_time = 0.0F;
                    }
                }
            });
        }};

        world.ecs.system<MoveTo>("move_target")
            .kind(world.fixed_phase)
            .run(move_target_system);

        world.ecs.system<MoveTo, WorldTransform, Animation>("move_to")
            .kind(world.fixed_phase)
            .each(move_to_system);

        world.ecs.system<Spin, WorldTransform>("spin")
            .kind(world.fixed_phase)
            .each(spin_system);

        world.ecs.system<Bounce, WorldTransform>("bounce")
            .kind(world.fixed_phase)
            .each(bounce_system);

        world.ecs.system<Consumer, WorldTransform, Animation>("eat_system")
            .kind(world.fixed_phase)
            .each(eat_system);
    }
}
