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
  PbrMaterial.cpp
  )

target_link_libraries(assets PUBLIC ecs engine)

if (WIN32)
  target_link_libraries(assets PRIVATE unofficial::gl3w::gl3w glfw glm::glm)
endif()
if (UNIX)
target_link_libraries(assets PRIVATE GLEW::glew glfw glm::glm OpenGL::GL)
endif()
