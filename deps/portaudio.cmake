set(PORTAUDIO_FOLDER ./portaudio)

add_library(portaudio SHARED IMPORTED GLOBAL)

target_include_directories(portaudio INTERFACE ${PORTAUDIO_FOLDER}/include)

set_target_properties(portaudio PROPERTIES IMPORTED_LOCATION ${PORTAUDIO_FOLDER})
