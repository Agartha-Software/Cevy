##
## Agartha-Software, 2023
## Cevy
## File description:
## assets-cmake
##

add_library(assets
  STATIC
  AssetManager.cpp
  Asset.hpp
  Handle.hpp
  AssetManager.hpp
  Model.cpp
  primitives.cpp
  PbrMaterial.cpp
  )

target_include_directories(assets PUBLIC .)

target_link_libraries(assets PUBLIC ecs engine cevy_headers)

if (WIN32)
  target_link_libraries(assets PRIVATE unofficial::gl3w::gl3w glfw glm::glm)
endif()
if (UNIX)
target_link_libraries(assets PRIVATE GLEW::glew glfw glm::glm OpenGL::GL)
endif()
