#include <flecs.h>
#include <raylib.h>
#include "world/world.h"

#include "world/systems/particle.h"
#include "world/systems/interpolation.h"
#include "world/systems/render.h"
#include "world/systems/gameplay.h"

auto World::create_world() -> World {
    const flecs::world ecs;

    const auto fixed_phase { ecs.entity("fixed_phase") };
    const auto render_phase { ecs.entity("render_phase") };

    // Game loop pipeline with a fixed interval of 60 FPS
    const auto fixed_pipeline { ecs.pipeline()
        .with(flecs::System)
        .with(fixed_phase)
        .build()
    };

    // Render pipeline without fixed interval
    const auto render_pipeline { ecs.pipeline()
        .with(flecs::System)
        .with(render_phase)
        .build()
    };

    auto world { World {
        .ecs = ecs,
        .fixed_pipeline = fixed_pipeline,
        .render_pipeline = render_pipeline,
        .fixed_phase = fixed_phase,
        .render_phase = render_phase,
        .accumulator = 0.0f,
    }};

    interpolation_systems::register_systems(world);
    gameplay_systems::register_systems(world);
    render_systems::register_systems(world);
    particle_systems::register_systems(world);

    return world;
}

auto World::update() -> void {
    const float dt { GetFrameTime() };
    accumulator += dt;

    // ReSharper disable once CppDFALoopConditionNotUpdated
    while (accumulator >= FIXED_DT) {
        ecs.run_pipeline(fixed_pipeline, FIXED_DT);
        accumulator -= FIXED_DT;
    }

    const float alpha { accumulator / FIXED_DT };
    ecs.run_pipeline(render_pipeline, alpha);
}