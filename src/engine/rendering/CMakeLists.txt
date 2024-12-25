##
## Agartha-Software, 2023
## Cevy
## File description:
## rendering-cmake
##

add_library(rendering
  STATIC
  Color.cpp
  ClearColor.cpp
  )

target_link_libraries(rendering PUBLIC ecs engine math)

target_include_directories(rendering PUBLIC .)

if (WIN32)
  target_link_libraries(rendering PRIVATE unofficial::gl3w::gl3w glfw glm::glm)
endif()
if (UNIX)
target_link_libraries(rendering PRIVATE GLEW::glew glfw glm::glm OpenGL::GL)
endif()
