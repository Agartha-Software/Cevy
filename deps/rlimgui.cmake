if (NOT EXISTS ${CMAKE_BINARY_DIR}/_deps/rlimgui) # If there's none, fetch and build rlimgui
  FetchContent_Declare(
      rlimgui
      DOWNLOAD_EXTRACT_TIMESTAMP OFF
      GIT_REPOSITORY https://github.com/raylib-extras/rlImGui
      GIT_TAG main
      SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/rlimgui
    )

  FetchContent_MakeAvailable(rlimgui)
endif()

set(RLIMGUI_FOLDER ${CMAKE_BINARY_DIR}/_deps/rlimgui)
set(IMGUI_FOLDER ${CMAKE_BINARY_DIR}/_deps/imgui)

add_library(rlImGui STATIC
  ${RLIMGUI_FOLDER}/rlImGui.cpp
)

target_include_directories(rlImGui PUBLIC
  ${RLIMGUI_FOLDER}
  ${IMGUI_FOLDER}
  ${IMGUI_FOLDER}/backends
)

target_compile_definitions(rlImGui PUBLIC NO_FONT_AWESOME)
target_link_libraries(rlImGui PUBLIC imgui raylib)
