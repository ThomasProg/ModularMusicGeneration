cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23")

FILE(GLOB SOURCES Private/*)
set(LibName "ModularMusicGenerationCore")
project(${LibName})

# Create library
add_library(${LibName} SHARED ${SOURCES})

# Add EasyMidiFileParserCpp
add_compile_definitions(MAKE_DLL)
add_subdirectory(ThirdParty/EasyMidiFileParserCpp)
target_link_libraries(${LibName} EasyMidiFileParserCpp)

# Add fluidsynth
set(enable-pkgconfig false) 
set(BUILD_SHARED_LIBS ON)
target_link_libraries(${LibName} "${PROJECT_SOURCE_DIR}/ThirdParty/fluidsynth/lib/libfluidsynth.dll.a")
target_compile_definitions(${LibName} PRIVATE MMG_EXPORT)
target_include_directories(${LibName} PUBLIC 
    "${PROJECT_SOURCE_DIR}/Public/" 
    "${PROJECT_SOURCE_DIR}/ThirdParty/fluidsynth/include"
    "${PROJECT_SOURCE_DIR}/ThirdParty/EasyMidiFileParserCpp/Public"
)

add_custom_command(TARGET ${LibName}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory 
  "${PROJECT_SOURCE_DIR}/ThirdParty/fluidsynth/bin/" 
  ${ModularMusicGenerationCore_BINARY_DIR}/$<CONFIG>/.
)

add_custom_command(TARGET ${LibName}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory 
  ${EasyMidiFileParserCpp_BINARY_DIR}/$<CONFIG>/.
  ${ModularMusicGenerationCore_BINARY_DIR}/$<CONFIG>/.
)
