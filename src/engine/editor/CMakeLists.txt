##
## Agartha-Software, 2023
## Cevy
## File description:
## editor-cmake
##

add_library(editor
  STATIC
  Editor.cpp
  EditorWindow.cpp
  GameWindow.cpp
  ProfilingWindow.cpp
  LoggingWindow.cpp
)

add_subdirectory(LegitProfiler)

target_include_directories(editor PUBLIC .)

target_link_libraries(editor PUBLIC engine ecs input legit imgui)
