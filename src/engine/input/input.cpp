/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Input
*/

#include "Event.hpp"
#include "Resource.hpp"
#include "state.hpp"
#include "input.hpp"

void update_input(
    cevy::ecs::EventReader<cevy::input::keyPressed> keyPressedReader,
    cevy::ecs::EventReader<cevy::input::keyReleased> keyReleasedReader,
    cevy::ecs::EventReader<cevy::input::cursorMoved> cursorMovedReader,
    cevy::ecs::EventWriter<cevy::input::mouseMotion> mouseMotionWriter,
    cevy::ecs::Resource<cevy::input::ButtonInput<cevy::input::KeyCode>> keyboard,
    cevy::ecs::Resource<cevy::input::cursorPosition> cursorPosition) {
    keyboard->clear();

    for (const auto &cursorMoved: cursorMovedReader) {
        mouseMotionWriter.send(cevy::input::mouseMotion {cursorPosition->pos - cursorMoved.pos });
        cursorPosition->pos = cursorMoved.pos;
    }

    for (const auto &pressed : keyPressedReader) {
        keyboard->press(pressed.keycode);
    }
    for (const auto &released : keyReleasedReader) {
        keyboard->release(released.keycode);
    }
}

void cevy::input::InputPlugin::build(cevy::ecs::App &app) {
    app.add_event<keyPressed>();
    app.add_event<keyReleased>();
    app.add_event<mouseMotion>();
    app.add_event<cursorMoved>();
    app.init_resource<ButtonInput<KeyCode>>(ButtonInput<KeyCode>());
    app.init_resource<cevy::input::cursorPosition>(cevy::input::cursorPosition{{0 , 0}});
    app.add_systems<ecs::core_stage::PreUpdate>(update_input);
}
