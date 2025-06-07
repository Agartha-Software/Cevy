##
## Agartha-Software, 2024
## Cevy
## File description:
## physics-cmake
##

add_library(physics
    STATIC
    Physics.cpp
  )

# add_subdirectory(collision)

target_include_directories(physics PUBLIC .)
target_include_directories(physics PUBLIC collision)

target_link_libraries(physics PUBLIC ecs engine)
