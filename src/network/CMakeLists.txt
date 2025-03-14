##
## Agartha-Software, 2023
## Cevy
## File description:
## network cmakelist
##

add_library(network
  STATIC
    NetworkPlugin.cpp
)

set(ASIO_FOLDER ${CMAKE_BINARY_DIR}/_deps/asio)

target_link_libraries(network PUBLIC ecs commands cevy_headers)
target_link_libraries(network PRIVATE asio)

target_include_directories(network PUBLIC ./ )
