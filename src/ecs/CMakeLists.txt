##
## Agartha-Software, 2023
## Cevy
## File description:
## ecs-cmake
##

add_library(ecs
STATIC
  ecs.cpp
  SparseVector.cpp
  Entity.cpp
  World.cpp
  Scheduler.cpp
  DefaultPlugin.cpp
  Time.cpp
  App.cpp
)

target_include_directories(ecs PUBLIC .)

target_link_libraries(ecs PUBLIC cevy_headers)


add_subdirectory(commands)

target_link_libraries(ecs PUBLIC commands )
