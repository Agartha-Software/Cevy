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
  Diffuse.hpp
  Handle.hpp
  AssetManager.hpp
  )

target_link_libraries(assets PUBLIC ecs engine)
