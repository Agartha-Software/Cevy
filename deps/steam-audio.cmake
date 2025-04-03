FetchContent_Declare(
  steam-audio
  DOWNLOAD_EXTRACT_TIMESTAMP OFF
  URL https://github.com/ValveSoftware/steam-audio/releases/download/v4.6.0/steamaudio_4.6.0.zip
  SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/steam-audio
)
