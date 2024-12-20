##
## Agartha-Software, 2023
## Cevy
## File description:
## rendering-cmake
##

add_library(rendering
  STATIC
  rendering.cpp
  Color.cpp
  ClearColor.cpp
  )

target_link_libraries(rendering PUBLIC raylib ecs engine math)
