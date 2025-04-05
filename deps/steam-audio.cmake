FetchContent_Declare(
  steam-audio
  DOWNLOAD_EXTRACT_TIMESTAMP OFF
  URL https://github.com/ValveSoftware/steam-audio/releases/download/v4.6.0/steamaudio_4.6.0.zip
  SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/steam-audio
)

FetchContent_GetProperties(steam-audio)
if(NOT steam-audio_POPULATED)
 FetchContent_MakeAvailable(steam-audio)
endif()

set(STEAM_AUDIO_FOLDER ${CMAKE_BINARY_DIR}/_deps/steam-audio)

# More complex conditional linkage should be added but for now the main support is for linux and windows
if (UNIX)
  set(STEAM_AUDIO_LIB_FILE ${STEAM_AUDIO_FOLDER}/lib/linux-x64//libphonon.so)
elseif (WIN32)
  set(STEAM_AUDIO_LIB_FILE ${STEAM_AUDIO_FOLDER}/lib/windows-x64/phonon.dll)
endif()

add_library(phonon SHARED IMPORTED GLOBAL)

target_include_directories(phonon INTERFACE ${STEAM_AUDIO_FOLDER}/include)

set_target_properties(phonon PROPERTIES IMPORTED_LOCATION ${STEAM_AUDIO_LIB_FILE})
