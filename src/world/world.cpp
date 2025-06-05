#include <flecs.h>
#include <raylib.h>
#include "world/world.h"
#include "world/systems/render.h"
#include "world/systems/gameplay.h"


constexpr float FIXED_DT = 1.0F / 60.0F;


auto World::create_world() -> World {
    flecs::world ecs;

    auto fixed_phase = ecs.entity("fixed_phase");
    auto render_phase = ecs.entity("render_phase");

    auto fixed_pipeline = ecs.pipeline()
        .with(flecs::System)
        .with(fixed_phase)
        .build();
    
    auto render_pipeline = ecs.pipeline()
        .with(flecs::System)
        .with(render_phase)
        .build();

    auto world = World {
        .ecs = ecs,
        .fixed_pipeline = fixed_pipeline,
        .render_pipeline = render_pipeline,
        .fixed_phase = fixed_phase,
        .render_phase = render_phase,
        .accumulator = 0.0F,
    };

    gameplay_systems::register_systems(world);
    render_systems::register_systems(world);

    return world;
}

auto World::update() -> void {

    float dt = GetFrameTime();
    accumulator += dt;
    
    while (accumulator >= FIXED_DT) {
        ecs.run_pipeline(fixed_pipeline, dt);
        accumulator -= FIXED_DT;
    }

    ecs.run_pipeline(render_pipeline, GetFrameTime());
}