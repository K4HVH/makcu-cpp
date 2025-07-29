cmake_minimum_required(VERSION 3.15)
project(makcu-examples)

# Find the makcu-cpp library
find_package(makcu-cpp REQUIRED)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Basic usage example
add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE makcu::makcu-cpp)

# Full demo application (original main.cpp)
add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE makcu::makcu-cpp)

# Set output directory
set_target_properties(basic_usage demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)