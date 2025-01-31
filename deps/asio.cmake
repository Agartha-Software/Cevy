FetchContent_Declare(
    asio
    GIT_REPOSITORY      https://github.com/chriskohlhoff/asio.git
    GIT_TAG             master
    # CONFIGURE_COMMAND   ""
    # BUILD_COMMAND       ""
    SOURCE_DIR          ${CMAKE_BINARY_DIR}/_deps/asio
)

FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
 FetchContent_MakeAvailable(asio)
endif()

set(ASIO_FOLDER ${CMAKE_BINARY_DIR}/_deps/asio)

add_library(asio INTERFACE)

target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
