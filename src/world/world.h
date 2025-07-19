#pragma once
#include <flecs.h>

constexpr float FIXED_DT { 1.0f / 60.0f };

class World {
    public:
        flecs::world ecs;
        flecs::entity pre_fixed_pipeline;
        flecs::entity fixed_pipeline;
        flecs::entity pre_render_pipeline;
        flecs::entity render_pipeline;

        flecs::entity pre_fixed_phase;
        flecs::entity fixed_phase;
        flecs::entity pre_render_phase;
        flecs::entity render_phase;

        float accumulator;

        static auto create_world() -> World;
        void update();
};

