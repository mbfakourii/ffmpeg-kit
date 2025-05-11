# Set minimum CMake version and project name
cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME "ffmpeg_kit_flutter")

# Define the project and specify C++ as the language
project(${PROJECT_NAME} LANGUAGES CXX)

# Set the CMake policy version for compatibility
cmake_policy(VERSION 3.14...3.25)

# Define the plugin name
set(PLUGIN_NAME "ffmpeg_kit_flutter_plugin")

# Add all .cpp files from the src directory to the source list
file(GLOB SRC_FILES "src/*.cpp")
list(APPEND PLUGIN_SOURCES ${SRC_FILES})

# Add compile definitions for FFmpeg Kit
add_compile_definitions(FFMPEG_KIT_LTS=0)
add_compile_definitions(FFMPEG_KIT_BUILD_DATE=20250503)

# Define the plugin library target as a shared library
add_library(${PLUGIN_NAME} SHARED
        "include/ffmpeg_kit_flutter/ffmpeg_kit_flutter_plugin_c_api.h"
        "ffmpeg_kit_flutter_plugin_c_api.cpp"
        "ffmpeg_kit_flutter_plugin.cpp"
        "ffmpeg_kit_flutter_plugin.h"
        ${PLUGIN_SOURCES}
)

# Apply standard build settings from the application-level CMake
apply_standard_settings(${PLUGIN_NAME})

# Define plugin-specific compile definitions for Flutter plugin
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)

# Suppress C++17 deprecation warnings related to rapidjson
target_compile_definitions(${PLUGIN_NAME} PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

# Include directories for rapidjson and plugin-specific headers
target_include_directories(${PLUGIN_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Define source include directories for the plugin
target_include_directories(${PLUGIN_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${PLUGIN_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Link the plugin to the Flutter and flutter_wrapper_plugin libraries
target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin)

# Specify libraries to be bundled with the plugin
set(ffmpeg_kit_flutter_bundled_libraries
        "${CMAKE_CURRENT_SOURCE_DIR}/bin/"
        PARENT_SCOPE
)