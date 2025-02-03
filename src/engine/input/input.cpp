/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Input
*/

#include "Event.hpp"
#include "Resource.hpp"
#include "state.hpp"
#include <glm/fwd.hpp>
#include <glm/gtx/string_cast.hpp>
#include <optional>
#include "input.hpp"

void update_mouse_button_input(
  cevy::ecs::EventReader<cevy::input::mouseInput> mouseInputReader,
  cevy::ecs::Resource<cevy::input::ButtonInput<cevy::input::MouseButton>> mouse) {
  mouse->clear();

  for (const auto &input : mouseInputReader) {
    if (input.pressed) {
      mouse->press(input.button);
    } else {
      mouse->release(input.button);
    }
  }
}

void update_mouse_motion_window_focus(
  cevy::ecs::EventReader<cevy::input::cursorMoved> cursorMovedReader,
  cevy::ecs::EventReader<cevy::input::windowFocused> windowFocusedReader,
  cevy::ecs::EventWriter<cevy::input::mouseMotion> mouseMotionWriter,
  cevy::ecs::Resource<cevy::input::windowFocus> windowFocusState,
  cevy::ecs::Resource<cevy::input::cursorPosition> cursorPosition
) {
  for (const auto &windowFocusChange: windowFocusedReader) {
    windowFocusState->focused = windowFocusChange.focused;
    cursorPosition->delta = std::nullopt;
  }

  for (const auto &cursorMoved: cursorMovedReader) {
    std::optional<glm::vec<2, int>> delta;

    if (cursorPosition->delta.has_value()) {
      delta = cursorMoved.pos - cursorPosition->pos;
    } else {
      delta = { 0, 0};
    }

    mouseMotionWriter.send(cevy::input::mouseMotion {
      .pos = cursorPosition->pos,
      .delta = delta
    });
    cursorPosition->pos = cursorMoved.pos;
    cursorPosition->delta = delta;
  }
}

void update_keyboard_input(
  cevy::ecs::EventReader<cevy::input::keyboardInput> keyboardInputReader,
  cevy::ecs::Resource<cevy::input::ButtonInput<cevy::input::KeyCode>> keyboard) {
  keyboard->clear();

  for (const auto &input : keyboardInputReader) {
    if (input.pressed) {
      keyboard->press(input.keycode);
    } else {
      keyboard->release(input.keycode);
    }
  }
}

void cevy::input::InputPlugin::build(cevy::ecs::App &app) {
  app.add_event<keyboardInput>();
  app.add_event<mouseInput>();
  app.add_event<mouseMotion>();
  app.add_event<cursorMoved>();
  app.add_event<windowFocused>();
  app.add_event<cursorEntered>();
  app.add_event<cursorLeft>();
  app.init_resource<ButtonInput<KeyCode>>(ButtonInput<KeyCode>());
  app.init_resource<ButtonInput<MouseButton>>(ButtonInput<MouseButton>());
  app.init_resource<windowFocus>(windowFocus { true});
  app.init_resource<cevy::input::cursorPosition>(cevy::input::cursorPosition{{0 , 0}, std::nullopt});
  app.add_systems<ecs::core_stage::PreUpdate>(update_mouse_motion_window_focus);
  app.add_systems<ecs::core_stage::PreUpdate>(update_keyboard_input);
  app.add_systems<ecs::core_stage::PreUpdate>(update_mouse_button_input);
}
