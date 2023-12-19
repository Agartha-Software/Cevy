/*
** Agartha-Software, 2023
** Cevy
** File description:
** Engine
*/

#include "Engine.hpp"
#include "World.hpp"
#include "ecs.hpp"
#include "raylib.h"
#include "rlImGui.h"
#include "AssetManager.hpp"
#include "Schedule.hpp"
#include "App.hpp"
#include "DefaultPlugin.hpp"
#include "Resource.hpp"
#include "Camera.hpp"
#include "Position.hpp"
#include "Commands.hpp"
#include "EntityCommands.hpp"
#include "imgui.h"

void init_window() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 450, "raylib [core] example - basic window");
    SetTargetFPS(60);
    rlImGuiSetup(true);
}

void close_game(cevy::ecs::Resource<struct cevy::ecs::Control> control) {
    if (WindowShouldClose())
        control.get().abort = true;
}

void update_window(cevy::ecs::Query<cevy::Camera> cams, cevy::ecs::Query<cevy::Position, cevy::Handle<cevy::Model3D>> models) {
    for (auto cam : cams) {
        UpdateCamera(std::get<0>(cam), CAMERA_FIRST_PERSON);
    }
    BeginDrawing();

    ClearBackground(WHITE);
    for (auto cam : cams) {
        BeginMode3D(std::get<0>(cam));
        for (auto model : models) {
            cevy::Position &pos = std::get<0>(model);
            cevy::Handle<cevy::Model3D> handle = std::get<1>(model);

            DrawModel(handle.get().model, Vector3 {(float)pos.x, (float)pos.y, (float)pos.z}, 2, WHITE);
        }
        DrawGrid(100, 1.0f);

        EndMode3D();
    }
    EndDrawing();
}

void cevy::Engine::build(cevy::ecs::App& app) {
    app.add_plugins(cevy::ecs::DefaultPlugin());
    app.add_stage<StartupRenderStage>();
    app.add_stage<PreStartupRenderStage>();
    app.add_stage<PostStartupRenderStage>();
    app.add_stage<RenderStage>();
    app.add_stage<PreRenderStage>();
    app.add_stage<PostRenderStage>();
    app.add_plugins(cevy::AssetManagerPlugin());
    app.add_system<cevy::PreStartupRenderStage>(init_window);
    app.add_system<cevy::PreRenderStage>(close_game);
    app.add_system<cevy::RenderStage>(update_window);
    app.init_component<cevy::Camera>();
    app.init_component<cevy::Position>();
    app.spawn(cevy::Camera());
}


/*
cevy::Engine::Engine() {
}

void cevy::Engine::DebugWindow (void) {
    if (IsKeyPressed(KEY_F10)) {
        debugMode = (debugMode == false);
        if (debugMode)
            EnableCursor();
    }
    if(debugMode) {
        rlImGuiBegin();
        if(!ImGui::Begin("Test WIndow", &debugMode)) {
            ImGui::End();
        } else {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();
        }
        rlImGuiEnd();
    } else {
        DisableCursor();
    }
}

void cevy::Engine::Fullscreen (void) {
    if (IsWindowFullscreen()) {
        int display = GetCurrentMonitor();
        this->screenWidth = GetMonitorWidth(display);
        this->screenHeight = GetMonitorHeight(display);
    } else {
        this->screenWidth = GetScreenWidth();
        this->screenHeight = GetScreenHeight();
    }
    if (!IsKeyPressed(KEY_F11))
        return;
    SetWindowSize(this->screenWidth, this->screenWidth);
 	ToggleFullscreen();
}

void cevy::Engine::update(void) {
    const char* text = "Your first window!";
    Vector2 text_size;

    while (!WindowShouldClose()) {
        Fullscreen();
        text_size = MeasureTextEx(GetFontDefault(), text, 20, 1);

        BeginDrawing();

        ClearBackground(DARKGRAY);
        DrawText(text, this->screenWidth / 2 - text_size.x / 2, this->screenHeight / 2 - text_size.y / 2, 20, RAYWHITE);
        BeginMode3D(camera);

        DrawGrid(100, 1.0f);

        EndMode3D();
        DebugWindow();
        EndDrawing();
    }
}
*/