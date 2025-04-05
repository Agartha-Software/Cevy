##
## Agartha-Software, 2025
## Cevy
## File description:
## audio-cmake
##

add_library(audio
  STATIC
  audio.cpp
)

target_include_directories(audio PUBLIC .)

target_link_libraries(audio PUBLIC engine ecs phonon)


