#include <flecs.h>
#include <raylib.h>
#include "world/world.h"

#include "world/systems/particle.h"
#include "world/systems/interpolation.h"
#include "world/systems/render.h"
#include "world/systems/gameplay.h"

#include <algorithm>

constexpr auto MAX_FRAME_TIME { 2.0f };

auto World::create_world() -> World {
    const flecs::world ecs;

    const auto fixed_phase { ecs.entity("fixed_phase") };
    const auto render_phase { ecs.entity("render_phase") };
    const auto pre_fixed_phase { ecs.entity("pre_render_phase") };
    const auto pre_render_phase { ecs.entity("pre_render_phase") };

    // Game loop pipeline with a fixed interval of 60 FPS
    const auto pre_fixed_pipeline { ecs.pipeline()
        .with(flecs::System)
        .with(pre_fixed_phase)
        .build()
    };
    const auto fixed_pipeline { ecs.pipeline()
        .with(flecs::System)
        .with(fixed_phase)
        .build()
    };

    // Render pipeline without fixed interval
    const auto render_pipeline { ecs.pipeline()
        .with(flecs::System)
        .with(pre_fixed_phase)
        .build()
    };

    const auto pre_render_pipeline { ecs.pipeline()
        .with(flecs::System)
        .with(render_phase)
        .build()
    };

    auto world { World {
        .ecs { ecs },
        .pre_fixed_pipeline { pre_fixed_pipeline },
        .fixed_pipeline { fixed_pipeline },
        .pre_render_pipeline { pre_render_pipeline },
        .render_pipeline { render_pipeline },
        .pre_fixed_phase { pre_fixed_phase },
        .fixed_phase { fixed_phase },
        .pre_render_phase { pre_render_phase },
        .render_phase { render_phase },
        .accumulator { 0.0f },
    }};

    interpolation_systems::register_systems(world);
    gameplay_systems::register_systems(world);
    render_systems::register_systems(world);
    particle_systems::register_systems(world);

    return world;
}

auto World::update() -> void {
    const float dt = std::min(GetFrameTime(), MAX_FRAME_TIME);
    accumulator += dt;

    // ReSharper disable once CppDFALoopConditionNotUpdated
    while (accumulator >= FIXED_DT) {
        ecs.run_pipeline(pre_fixed_pipeline, FIXED_DT);
        ecs.run_pipeline(fixed_pipeline, FIXED_DT);
        accumulator -= FIXED_DT;
    }

    const float alpha { accumulator / FIXED_DT };
    ecs.run_pipeline(pre_render_pipeline, alpha);
    ecs.run_pipeline(render_pipeline, alpha);
}