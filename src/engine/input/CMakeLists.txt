##
## Agartha-Software, 2023
## Cevy
## File description:
## input-cmake
##

add_library(input
  STATIC
  input.cpp
  input.hpp
  state.hpp
  )

target_include_directories(input PUBLIC
  ..
)

target_link_libraries(input PUBLIC engine ecs)

