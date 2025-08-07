#include <flecs.h>
#include <raylib.h>
#include <raymath.h>
#include "world/components/gameplay.h"
#include "world/components/render.h"
#include "world/components/particle.h"
#include "world/world.h"
#include "world/terrain/terrain.h"

constexpr auto max_turn { 7.5f };

namespace gameplay_systems {
    void register_systems(const World &world) {
        // Sets the MoveTo component to where the player clicks
        const auto move_target_system { [](flecs::iter &iter) {
            const auto should_move { IsMouseButtonDown(MOUSE_LEFT_BUTTON) };
            const auto *cam { should_move ? iter.world().get_mut<WorldCamera>() : nullptr };

            while (iter.next()) {
                if (!should_move || !cam) continue;

                auto move_to { iter.field<MoveTo>(0) };
                const auto ray { GetMouseRay(GetMousePosition(), cam->camera) };

                if (const auto hit { terrain::ray_ground_intersect(ray.position, ray.direction) }) {
                    for (const auto i : iter) {
                        move_to[i].path.clear();
                        move_to[i].waypoint = 0;
                        terrain::find_path(cam->camera.target, *hit, move_to[i].path);
                    }
                }
            }
        }};

        // Moves an animated entity with a transform towards MoveTo
        const auto move_to_system { [](const MoveTo &move_to, WorldTransform &transform, Animation &animation) {
            if (move_to.path.empty() || move_to.waypoint >= move_to.path.size()) {
                animation.name = "Idle";
                return;
            }

            auto target { move_to.path[move_to.waypoint] };
            auto direction { Vector3Subtract(target, transform.pos) };

            if (Vector2Length({direction.x, direction.z}) < 0.5f) {
                const_cast<MoveTo&>(move_to).waypoint++;
                if (move_to.waypoint >= move_to.path.size()) {
                    animation.name = "Idle";
                    return;
                }
                target = move_to.path[move_to.waypoint];
                direction = Vector3Subtract(target, transform.pos);
            }

            const Vector3 forward { Vector3Normalize(direction) };
            transform.pos = Vector3Add(transform.pos, Vector3Scale(forward, move_to.speed));

            // Smooth turning
            const auto target_angle { atan2f(forward.x, forward.z) * (180.0f / PI) };
            auto angle_diff { target_angle - transform.rot.y };
            while (angle_diff > 180.0f) angle_diff -= 360.0f;
            while (angle_diff < -180.0f) angle_diff += 360.0f;

            if (fabsf(angle_diff) <= max_turn) {
                transform.rot.y = target_angle;
            } else {
                transform.rot.y += (angle_diff > 0) ? max_turn : -max_turn;
            }

            animation.name = "Run";
            transform.pos.y = terrain::get_height(transform.pos.x, transform.pos.z);
        }};

        // Make an entity spin
        const auto spin_system { [](const Spin &spin, WorldTransform &transform) {
            transform.rot.y += spin.speed;
        }};

        // Makes an entity bounce
        const auto bounce_system { [](Bounce &bounce, WorldTransform &transform) {
            transform.pos.y = terrain::get_height(transform.pos.x, transform.pos.z) + bounce.center_y + sinf(bounce.elapsed) * bounce.height;
            bounce.elapsed += bounce.speed;
        }};

        const auto eat_system { [&ecs = world.ecs](const Consumer &consumer, const WorldTransform &transform, Animation &animation) {
            ecs.each([&](const flecs::entity consumable_entity, const Consumable &consumable, const WorldTransform &consumable_transform) {
                const float distance = Vector2Distance(
                    { .x = transform.pos.x, .y = transform.pos.z },
                    { .x = consumable_transform.pos.x, .y = consumable_transform.pos.z
                });

                if (distance <= consumer.range && !animation.run_once.has_value()) {
                    consumable_entity.destruct();
                    ecs.entity()
                        .set<WorldTransform>(consumable_transform)
                        .set<Explosion>({
                            .particles { consumable.particles },
                            .colors { consumable.colors },
                        });

                    animation.run_once = "Eat";
                    animation.frame_time = 0.0f;
                }
            });
        }};

        const auto collision_system { [&world](flecs::iter& iter) {
            auto has_changed { false };

            while (iter.next()) {
                if (iter.changed()) {
                    has_changed = true;
                }
            }

            if (has_changed) {
                terrain::update_collision_entities(world.ecs);
            }
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

        world.ecs.system<Consumer, WorldTransform, Animation>("eat")
            .kind(world.fixed_phase)
            .each(eat_system);

        world.ecs.system<Collider, WorldTransform>("collision")
            .kind(world.fixed_phase)
            .cached()
            .run(collision_system);
    }
}
