cmake_minimum_required(VERSION 3.15)
project(makcu-examples)

# Find the makcu-cpp library
find_package(makcu-cpp REQUIRED)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# C standard for C API test
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Basic usage example
add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE makcu::makcu-cpp)

# Full demo application (original main.cpp)
add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE makcu::makcu-cpp)

# C API test
add_executable(c_api_test c_api_test.c)
target_link_libraries(c_api_test PRIVATE makcu::makcu-cpp)

# Baud rate change test
add_executable(baud_rate_test baud_rate_test.cpp)
target_link_libraries(baud_rate_test PRIVATE makcu::makcu-cpp)

# Set output directory
set_target_properties(basic_usage demo c_api_test baud_rate_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)