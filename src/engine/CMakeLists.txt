##
## Agartha-Software, 2023
## Cevy
## File description:
## engine-cmake
##

set(IMGUI_FOLDER ${CMAKE_BINARY_DIR}/_deps/imgui)

add_library(engine
  STATIC
    Camera.cpp
    glWindow.cpp
)

add_subdirectory(math)
add_subdirectory(assets)
add_subdirectory(input)
add_subdirectory(editor)
add_subdirectory(rendering)

target_link_libraries(engine PUBLIC ecs math assets editor input rendering)

if (WIN32)
  target_link_libraries(engine PRIVATE unofficial::gl3w::gl3w glfw PUBLIC glm::glm)
endif()
if (UNIX)
  target_link_libraries(engine PRIVATE GLEW::glew glfw OpenGL::GL PUBLIC glm::glm)
endif()

target_include_directories(engine PUBLIC ./)

#target_include_directories(engine PUBLIC
#   ../ecs
#   ../ecs/commands
#   .
#  ./math
#  assets
#  input
#  rendering
#)

# target_include_directories(rlImGui PRIVATE
#   ${RLIMGUI_FOLDER}
#   ${IMGUI_FOLDER}
#   ${IMGUI_FOLDER}/backends
# )
