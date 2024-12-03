# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(audio_player_name_tbd VERSION 1.0.0)

# Set the C++ standard (C++11, C++17, etc.)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# Define the executable name
set(EXECUTABLE_NAME audio_player_name_tbd.out)

# Collect all the .cpp files in the src directory
file(GLOB SRC_FILES src/*.cpp)

# Add the executable with the source files
add_executable(${EXECUTABLE_NAME} ${SRC_FILES})

# Include directories (add your include directory)
target_include_directories(${EXECUTABLE_NAME} PRIVATE inc)

# Optional: Add extra flags (if needed)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Example of adding specific compiler options
target_compile_options(${EXECUTABLE_NAME} PRIVATE
    $<$<CONFIG:Debug>:-Wall -Wextra>
    $<$<CONFIG:Release>:-Wall -Wextra -O3>
)

# Example of linking libraries (if needed)
# target_link_libraries(${EXECUTABLE_NAME} PRIVATE some_library)