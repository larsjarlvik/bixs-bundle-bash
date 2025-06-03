#pragma once

#include "../components/rendering.hpp"
#include <raylib.h>
#include <flecs.h>

void begin_render(flecs::iter &iter);
void setup_lighting(flecs::entity entity, WorldShader &shader);
void render_model(flecs::entity entity, WorldModel &model, WorldShader &shader, WorldPos &pos);
void end_render(flecs::iter &iter);
