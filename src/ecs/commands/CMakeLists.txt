##
## Agartha-Software, 2023
## Cevy
## File description:
## commands-cmake
##

add_library(commands
  STATIC
  Commands.hpp
  Command.hpp
  Commands.cpp
  EntityCommands.hpp
  EntityCommands.cpp
  )

target_include_directories(commands PUBLIC ..)

target_link_libraries(commands PUBLIC ecs)
