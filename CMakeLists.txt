cmake_minimum_required(VERSION 3.15)
project(makcu-cpp VERSION 1.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type options
option(BUILD_SHARED_ONLY "Build only shared library" OFF)
option(BUILD_STATIC_ONLY "Build only static library" OFF)

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /Wall")
endif()

# Find required packages
find_package(Threads REQUIRED)

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    # Linux-specific libraries
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(UDEV REQUIRED libudev)
endif()

# Library source files
set(LIBRARY_SOURCES
    makcu-cpp/src/makcu.cpp
    makcu-cpp/src/serialport.cpp
)

# Headers
set(HEADERS
    makcu-cpp/include/makcu.h
    makcu-cpp/include/serialport.h
    makcu-cpp/include/makcu_export.h
)

# Create both shared and static libraries by default
set(LIBRARY_TARGETS)

# Build shared library (unless BUILD_STATIC_ONLY is set)
if(NOT BUILD_STATIC_ONLY)
    add_library(${PROJECT_NAME}_shared SHARED ${LIBRARY_SOURCES} ${HEADERS})
    set_target_properties(${PROJECT_NAME}_shared PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        PUBLIC_HEADER "${HEADERS}"
    )
    list(APPEND LIBRARY_TARGETS ${PROJECT_NAME}_shared)
endif()

# Build static library (unless BUILD_SHARED_ONLY is set)
if(NOT BUILD_SHARED_ONLY)
    add_library(${PROJECT_NAME}_static STATIC ${LIBRARY_SOURCES} ${HEADERS})
    set_target_properties(${PROJECT_NAME}_static PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        VERSION ${PROJECT_VERSION}
        PUBLIC_HEADER "${HEADERS}"
    )
    list(APPEND LIBRARY_TARGETS ${PROJECT_NAME}_static)
endif()

# Create alias for backward compatibility - prefer shared library
if(NOT BUILD_STATIC_ONLY)
    add_library(${PROJECT_NAME} ALIAS ${PROJECT_NAME}_shared)
else()
    add_library(${PROJECT_NAME} ALIAS ${PROJECT_NAME}_static)
endif()

# Configure all library targets
foreach(target ${LIBRARY_TARGETS})
    # Include directories for the library
    target_include_directories(${target} 
        PUBLIC 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/makcu-cpp/include>
            $<INSTALL_INTERFACE:include/makcu>
    )

    # Link libraries
    target_link_libraries(${target} PUBLIC Threads::Threads)

    # Platform-specific linking
    if(WIN32)
        target_link_libraries(${target} PUBLIC setupapi)
        target_compile_definitions(${target} PRIVATE _WIN32_WINNT=0x0601)
        
        # Export symbols for shared library on Windows
        if(${target} STREQUAL "${PROJECT_NAME}_shared")
            target_compile_definitions(${target} PRIVATE MAKCU_EXPORTS)
            target_compile_definitions(${target} INTERFACE MAKCU_SHARED)
        endif()
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(${target} PUBLIC ${UDEV_LIBRARIES})
        target_include_directories(${target} PUBLIC ${UDEV_INCLUDE_DIRS})
        target_compile_options(${target} PUBLIC ${UDEV_CFLAGS_OTHER})
    endif()

    # Set output directory
    set_target_properties(${target} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endforeach()


# Install targets
install(TARGETS ${LIBRARY_TARGETS}
    EXPORT makcu-cppTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/makcu
)


# Create and install CMake config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/makcu-cppConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

export(EXPORT makcu-cppTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/makcu-cppTargets.cmake"
    NAMESPACE makcu::
)

# Generate CMake config file inline
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/makcu-cppConfig.cmake" "
include(CMakeFindDependencyMacro)

# Find required dependencies
find_dependency(Threads REQUIRED)

# Platform-specific dependencies
if(UNIX AND NOT APPLE)
    find_dependency(PkgConfig REQUIRED)
    pkg_check_modules(UDEV REQUIRED libudev)
endif()

# Include the targets file
include(\"\${CMAKE_CURRENT_LIST_DIR}/makcu-cppTargets.cmake\")

# Create convenient alias - prefer shared library if available, fallback to static
if(TARGET makcu::makcu-cpp_shared)
    if(NOT TARGET makcu::makcu-cpp)
        add_library(makcu::makcu-cpp ALIAS makcu::makcu-cpp_shared)
    endif()
elseif(TARGET makcu::makcu-cpp_static)
    if(NOT TARGET makcu::makcu-cpp)
        add_library(makcu::makcu-cpp ALIAS makcu::makcu-cpp_static)
    endif()
endif()
")

install(EXPORT makcu-cppTargets
    FILE makcu-cppTargets.cmake
    NAMESPACE makcu::
    DESTINATION lib/cmake/makcu-cpp
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/makcu-cppConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/makcu-cppConfigVersion.cmake"
    DESTINATION lib/cmake/makcu-cpp
)

# Package configuration
set(CPACK_PACKAGE_NAME "makcu-cpp")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High-Performance C++ Library for MAKCU Mouse Controllers")
set(CPACK_PACKAGE_VENDOR "K4HVH")
set(CPACK_PACKAGE_CONTACT "https://github.com/K4HVH/makcu-cpp")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libudev1")
    set(CPACK_RPM_PACKAGE_REQUIRES "systemd-devel")
endif()

include(CPack)