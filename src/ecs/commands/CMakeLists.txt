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

include_directories(PUBLIC ..)
