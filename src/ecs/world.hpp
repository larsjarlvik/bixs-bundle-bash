#pragma once
#include <flecs.h>

class World {
    public:
        flecs::world ecs;
        
        static auto create_world() -> World;
        void update() const;
};
