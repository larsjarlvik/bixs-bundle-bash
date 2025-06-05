#include <flecs.h>
#include "world/world.h"
#include "world/systems/render_systems.h"
#include "world/systems/camera_systems.h"

auto World::create_world() -> World {
    flecs::world ecs;

    camera_systems::register_systems(ecs);
    render_systems::register_systems(ecs);

    World world;
    world.ecs = ecs;
    return world;
}

auto World::update() const -> void {
    ecs.progress();
}