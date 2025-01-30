/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Input
*/

#include "glWindow.hpp"
#include "Event.hpp"
#include "Resource.hpp"
#include "World.hpp"
#include "state.hpp"
#include "input.hpp"

void update_input(
    cevy::ecs::EventWriter<cevy::input::keyPressed> keyPressedWriter,
    cevy::ecs::EventWriter<cevy::input::keyReleased> keyReleasedWriter,
    cevy::ecs::Resource<cevy::engine::Window> window,
    cevy::ecs::Resource<cevy::input::ButtonInput<cevy::input::KeyCode>> keyboard_res,
    cevy::ecs::World &world) {
    auto &keyboard = keyboard_res.get();

    window.get()->setKeyPressedWriter(keyPressedWriter);
    window.get()->setKeyReleasedWriter(keyReleasedWriter);
    window.get()->pollEvents();
    window.get()->getKeyPressedWriter().reset();
    window.get()->getKeyReleasedWriter().reset();

    keyboard.clear();

    auto &event_pressed = world.resource<cevy::ecs::Event<cevy::input::keyPressed>>();
    for (auto &pressed : event_pressed.event_queue) {
        keyboard.press(std::get<0>(pressed).keycode);
    }

    auto &event_release = world.resource<cevy::ecs::Event<cevy::input::keyReleased>>();
    for (auto &release : event_release.event_queue) {
        keyboard.release(std::get<0>(release).keycode);
    }
}

void cevy::input::InputPlugin::build(cevy::ecs::App &app) {
    app.add_event<keyPressed>();
    app.add_event<keyReleased>();
    app.init_resource<ButtonInput<KeyCode>>(ButtonInput<KeyCode>());
    app.add_systems<ecs::core_stage::PreUpdate>(update_input);
}
