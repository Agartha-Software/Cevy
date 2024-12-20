##
## Agartha-Software, 2023
## Cevy
## File description:
## ecs-cmake
##

set(CMAKE_CXX_STANDARD 17)

add_library(SparseVector
  STATIC
  SparseVector.cpp
  )

add_library(entity
  STATIC
  Entity.cpp
  Entity.hpp
  )

target_link_libraries(entity PUBLIC SparseVector)

# include_directories(PUBLIC . ./commands ../)
# include_directories(PUBLIC .)

add_subdirectory(commands)

add_library(world
  STATIC
  World.cpp
  World.hpp
  )

add_library(schedule
  STATIC
  Scheduler.cpp
  Scheduler.hpp
  Stage.hpp
  )

add_library(default_plugin
  STATIC
  DefaultPlugin.cpp
  DefaultPlugin.hpp
  )

add_library(time
  STATIC
  Time.cpp
  Time.hpp
  )

add_library(app
  STATIC
  App.cpp
  App.hpp
  Plugin.hpp
  )

file(GLOB CEVY_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../../public/*")
add_library(ecs INTERFACE ${CEVY_HEADERS} ecs.hpp)
target_link_libraries(ecs INTERFACE SparseVector world commands app entity schedule default_plugin time)
set_target_properties(ecs PROPERTIES LINKER_LANGUAGE CXX)
