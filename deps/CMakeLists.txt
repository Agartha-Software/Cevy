include(FetchContent)

include("./raylib.cmake")
include("./imgui.cmake")
include("./rlimgui.cmake")
include("./asio.cmake")


find_package(glm CONFIG REQUIRED)

if (WIN32)
  find_package(gl3w CONFIG REQUIRED)
endif()
if (UNIX)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
endif()

find_package(glfw3 CONFIG REQUIRED)
