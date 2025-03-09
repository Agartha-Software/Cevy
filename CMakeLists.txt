##
## Agartha-Software, 2023
## Cevy
## File description:
## Cevy project cmake
##

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(Cevy LANGUAGES CXX VERSION 0.1)
set(Cevy_VERSION 0.1)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(deps)

find_package(glm CONFIG REQUIRED)

if (WIN32)
  find_package(gl3w CONFIG REQUIRED)
endif()
if (UNIX)
    find_package(GLEW REQUIRED)
    set(OpenGL_GL_PREFERENCE "GLVND")
    find_package(OpenGL REQUIRED)
endif()

find_package(glfw3 CONFIG REQUIRED)

if (UNIX)
add_compile_options(-Wall -Wextra)
endif()

option(DEBUG_MODE "Set DEBUG Mode" ON)

add_definitions(-mavx)

if(!DEBUG_MODE)
    add_definitions(-ffast-math -Ofast)
endif()

add_subdirectory(src)

option(TESTS "Build tests" OFF)
if(TESTS)
    add_custom_target(tests-run-cevy
                        COMMENT "Executing tests"
                        COMMAND test-cevy
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        SOURCES ${ToolsSources}
                    )

    add_subdirectory(test)
endif()
