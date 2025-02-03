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
#include <optional>
#include "input.hpp"

void update_input(
    cevy::ecs::EventReader<cevy::input::keyboardInput> keyboardInputReader,
    cevy::ecs::EventReader<cevy::input::cursorMoved> cursorMovedReader,
    cevy::ecs::EventReader<cevy::input::windowFocused> windowFocusedReader,
    cevy::ecs::EventWriter<cevy::input::mouseMotion> mouseMotionWriter,
    cevy::ecs::Resource<cevy::input::ButtonInput<cevy::input::KeyCode>> keyboard,
    cevy::ecs::Resource<cevy::input::windowFocus> windowFocusState,
    cevy::ecs::Resource<cevy::input::cursorPosition> cursorPosition) {
    keyboard->clear();

    for (const auto &cursorMoved: cursorMovedReader) {
        std::optional<glm::vec<2, int>> delta;

        if (windowFocusState->focused) {
            delta = cursorPosition->pos - cursorMoved.pos;
        } else {
            delta = std::nullopt;
        }

        mouseMotionWriter.send(cevy::input::mouseMotion {
            .pos = cursorPosition->pos,
            .delta = delta
        });
        cursorPosition->pos = cursorMoved.pos;
        cursorPosition->delta = delta;
    }

    for (const auto &input : keyboardInputReader) {
        if (input.pressed) {
            keyboard->press(input.keycode);
        } else {
            keyboard->release(input.keycode);
        }
    }

    for (const auto &windowFocusChange: windowFocusedReader) {
        windowFocusState->focused = windowFocusChange.focused;
    }
}

void cevy::input::InputPlugin::build(cevy::ecs::App &app) {
    app.add_event<keyboardInput>();
    app.add_event<mouseMotion>();
    app.add_event<cursorMoved>();
    app.add_event<windowFocused>();
    app.init_resource<ButtonInput<KeyCode>>(ButtonInput<KeyCode>());
    app.init_resource<windowFocus>(windowFocus { true});
    app.init_resource<cevy::input::cursorPosition>(cevy::input::cursorPosition{{0 , 0}, std::nullopt});
    app.add_systems<ecs::core_stage::PreUpdate>(update_input);
}
