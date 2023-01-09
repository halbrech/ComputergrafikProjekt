cmake_minimum_required(VERSION 3.13)  # CMake version check
project(Computergrafik)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

# Add main.cpp file of project root directory as source file
set(SOURCE_FILES src/main.cpp src/glad.c)

find_package(OpenGL REQUIRED)

include_directories(include)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(Computergrafik ${SOURCE_FILES})

target_link_libraries(Computergrafik PRIVATE OpenGL::GL glfw dl)
