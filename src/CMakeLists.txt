##
## Agartha-Software, 2023
## Cevy
## File description:
## cevy-cmake
##

add_library(cevy_headers STATIC cevy.cpp)
target_include_directories(cevy_headers PUBLIC ./)


add_subdirectory(ecs)
add_subdirectory(network)
add_subdirectory(engine)

## IN DEVELOPPEMENT:
## add_subdirectory(physics)

file(GLOB CEVY_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../public/*")
add_library(cevy SHARED ${CEVY_HEADERS})
target_link_libraries(cevy PUBLIC ecs engine) # physics)
set_target_properties(cevy PROPERTIES LINKER_LANGUAGE CXX)


function(get_cevy_engine_assets target)
add_custom_target(cevy_always
        COMMAND ${CMAKE_COMMAND} -E echo "cevy_always:" "`date`")
add_dependencies(${target} cevy_always)
add_custom_command(
    TARGET cevy_always POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_PROPERTY:cevy,SOURCE_DIR>/../assets/engine/"
        "$<TARGET_PROPERTY:${target},RUNTIME_OUTPUT_DIRECTORY>/assets/engine/"
    COMMAND
        echo "Copying engine assets to"
        "$<TARGET_PROPERTY:${target},RUNTIME_OUTPUT_DIRECTORY>/assets/engine/"

)
endfunction()

function(copy_cevy_engine_assets_to target directory)
add_custom_target(cevy_always
        COMMAND ${CMAKE_COMMAND} -E echo "cevy_always:" "`date`")
add_dependencies(${target} cevy_always)
add_custom_command(
    TARGET cevy_always POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_PROPERTY:cevy,SOURCE_DIR>/../assets/engine/"
        "${directory}/assets/engine/"
    COMMAND
        echo "Copying engine assets to" "${directory}/assets/engine/"
)
endfunction()

if(DEBUG_MODE)
    target_compile_definitions(engine
        INTERFACE DEBUG
    )
    target_compile_definitions(ecs
       INTERFACE DEBUG
    )
    ## target_compile_definitions(physics
    ##     INTERFACE DEBUG
    ## )
endif(DEBUG_MODE)
