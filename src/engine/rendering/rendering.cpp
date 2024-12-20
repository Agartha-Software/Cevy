/*
** Agartha-Software, 2023
** C++evy
** File description:
** Rendering
*/

#include "rendering.hpp"
#include "engine/Transform.hpp"
#include "Color.hpp"
#include "Diffuse.hpp"
#include "Handle.hpp"
#include "Line.hpp"
#include "Mesh.hpp"
#include "ecs/Query.hpp"
#include "ecs/World.hpp"
#include "cevy.hpp"
#include "ecs/ecs.hpp"
#include "raylib.h"
#include "raymath.h"

using namespace cevy::engine;
using namespace cevy;

void render_lines(cevy::ecs::World &w) {
  auto lines =
      ecs::Query<Line, option<cevy::engine::Transform>, option<cevy::engine::Color>>::query(w);
  for (auto [line, opt_transform, opt_color] : lines) {
    const cevy::engine::Transform &trans =
        opt_transform.value_or(cevy::engine::Transform(0., 0., 0.));
    const cevy::engine::Color &col = opt_color.value_or(cevy::engine::Color(0., 255., 60));
    Vector3 end = Vector3RotateByQuaternion(line.end - line.start, trans.rotation);
    end = Vector3Add(end, line.start + trans.position);
    DrawCylinderEx(line.start + trans.position, end, 0.1, 0.1, 4, (::Color)col);
  }
}

static void render_model(Model &model, engine::Transform transform, ::Color tint) {
  model.transform = QuaternionToMatrix(transform.rotation);
  model.transform = MatrixMultiply(
      model.transform, MatrixScale(transform.scale.x, transform.scale.y, transform.scale.z));
  for (int i = 0; i < model.meshCount; ++i) {
    DrawModelEx(model, transform.position, Vector3{0, 0, 0}, 1, Vector3{1, 1, 1}, tint);
  }
}

void render_models(cevy::ecs::World &w) {
  auto models = ecs::Query<option<engine::Transform>, Handle<engine::Mesh>, option<Handle<Diffuse>>,
                           option<engine::Color>>::query(w);

  for (auto [opt_tm, mesh, opt_diffuse, opt_color] : models) {
    const cevy::engine::Transform &tm = opt_tm.value_or(cevy::engine::Transform());
    auto handle = mesh.get();
    ::Color ray_color;
    if (opt_color) {
      ray_color = opt_color.value();
    } else
      ray_color = ::WHITE;
    if (opt_diffuse) {
      SetMaterialTexture(handle->mesh.materials, MATERIAL_MAP_DIFFUSE,
                         opt_diffuse.value().get()->texture);
    }
    render_model(handle->mesh, tm, ray_color);
    if (opt_diffuse) {
      handle->mesh.materialCount = 1;
      handle->mesh.materials = (Material *)RL_CALLOC(handle->mesh.materialCount, sizeof(Material));
      handle->mesh.materials[0] = LoadMaterialDefault();
    }
  }
}
