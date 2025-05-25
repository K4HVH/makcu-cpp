cmake_minimum_required(VERSION 3.16)
project(makcu-cpp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add compile options
if(MSVC)
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(makcu-cpp/include)

# Source files
set(MAKCU_SOURCES
    makcu-cpp/src/makcu.cpp
    makcu-cpp/src/serialport.cpp
    makcu-cpp/src/utilities.cpp
)

set(MAKCU_HEADERS
    makcu-cpp/include/makcu.h
    makcu-cpp/include/serialport.h
    makcu-cpp/include/utilities.h
)

# Create static library
add_library(makcu-static STATIC ${MAKCU_SOURCES} ${MAKCU_HEADERS})
target_include_directories(makcu-static PUBLIC makcu-cpp/include)

# Create shared library
add_library(makcu-shared SHARED ${MAKCU_SOURCES} ${MAKCU_HEADERS})
target_include_directories(makcu-shared PUBLIC makcu-cpp/include)

# Platform-specific linking
if(WIN32)
    target_link_libraries(makcu-static setupapi)
    target_link_libraries(makcu-shared setupapi)
endif()

# Set library properties
set_target_properties(makcu-static PROPERTIES
    OUTPUT_NAME makcu
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

set_target_properties(makcu-shared PROPERTIES
    OUTPUT_NAME makcu
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Main executable
add_executable(makcu-demo makcu-cpp/main.cpp)
target_link_libraries(makcu-demo makcu-static)

# Examples
add_executable(mouse-monitor examples/mouse_monitor.cpp)
target_link_libraries(mouse-monitor makcu-static)

add_executable(keyboard-automation examples/keyboard_automation.cpp)
target_link_libraries(keyboard-automation makcu-static)

add_executable(mouse-automation examples/mouse_automation.cpp)
target_link_libraries(mouse-automation makcu-static)

add_executable(advanced-features examples/advanced_features.cpp)
target_link_libraries(advanced-features makcu-static)

# Install targets
install(TARGETS makcu-static makcu-shared makcu-demo
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${MAKCU_HEADERS}
    DESTINATION include/makcu
)

# Package configuration
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/makcu-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/makcu-config.cmake"
    INSTALL_DESTINATION lib/cmake/makcu
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/makcu-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/makcu-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/makcu-config-version.cmake"
    DESTINATION lib/cmake/makcu
)

# Export targets
install(EXPORT makcu-targets
    FILE makcu-targets.cmake
    NAMESPACE makcu::
    DESTINATION lib/cmake/makcu
)

export(EXPORT makcu-targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/makcu-targets.cmake"
    NAMESPACE makcu::
)

# Testing (optional)
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Documentation (optional)
option(BUILD_DOCS "Build documentation" OFF)
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()

# CPack configuration
set(CPACK_PACKAGE_NAME "makcu-cpp")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ library for MAKCU devices")
set(CPACK_PACKAGE_VENDOR "MAKCU")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)
