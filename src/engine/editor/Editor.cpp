/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor
*/

#include "Editor.hpp"
#include "App.hpp"

void cevy::editor::EditorPlugin::build(cevy::ecs::App &app) {
    app.insert_resource(Editor {});
}
