#include <flecs.h>
#include "world/world.h"
#include "world/components/camera.h"
#include "world/components/rendering.h"
#include "world/systems/render_systems.h"
#include "world/systems/camera_systems.h"

auto World::create_world() -> World {
    flecs::world ecs;

    // Camera systems
    ecs.system("update_camera")
        .kind(flecs::OnUpdate)
        .with<WorldCamera>()
        .run(update_camera);
    
    ecs.system<WorldPos>("mouse_target")
        .kind(flecs::OnUpdate)
        .with<WorldMouseTarget>()
        .run(mouse_target);

    // Render systems
    ecs.system("begin_render")
        .kind(flecs::OnUpdate)
        .with<WorldCamera>()
        .run(begin_render);

    ecs.system<WorldShader>("setup_lighting")
        .kind(flecs::OnUpdate)
        .each(setup_lighting);
        
    ecs.system<WorldModel, WorldShader, WorldPos>("render_model")
        .kind(flecs::OnUpdate)
        .each(render_model);
            
    ecs.system("render_end")
        .kind(flecs::OnUpdate)
        .run(end_render);

    World world;
    world.ecs = ecs;
    return world;
}

auto World::update() const -> void {
    ecs.progress();
}