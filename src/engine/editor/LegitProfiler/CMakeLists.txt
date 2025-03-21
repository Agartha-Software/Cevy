##
## Agartha-Software, 2023
## Cevy
## File description:
## Legit-Profiler-cmake
##

add_library(legit
  STATIC
    ProfilerGraph.cpp
)

target_include_directories(legit PUBLIC
  .
)

target_link_libraries(legit PUBLIC imgui)

