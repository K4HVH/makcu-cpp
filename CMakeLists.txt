cmake_minimum_required(VERSION 3.15)
project(makcu-cpp VERSION 1.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

# Include directories
include_directories(makcu-cpp/include)

# Source files
set(SOURCES
    makcu-cpp/src/makcu.cpp
    makcu-cpp/src/serialport.cpp
    makcu-cpp/main.cpp
)

# Headers
set(HEADERS
    makcu-cpp/include/makcu.h
    makcu-cpp/include/serialport.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# Platform-specific linking
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE setupapi)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${UDEV_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${UDEV_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME} PRIVATE ${UDEV_CFLAGS_OTHER})
endif()

# Compiler definitions
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0601)
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install targets
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS}
    DESTINATION include/makcu
)

# Create a library version as well
add_library(makcu STATIC 
    makcu-cpp/src/makcu.cpp
    makcu-cpp/src/serialport.cpp
)

target_link_libraries(makcu PRIVATE Threads::Threads)

if(WIN32)
    target_link_libraries(makcu PRIVATE setupapi)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(makcu PRIVATE ${UDEV_LIBRARIES})
    target_include_directories(makcu PRIVATE ${UDEV_INCLUDE_DIRS})
    target_compile_options(makcu PRIVATE ${UDEV_CFLAGS_OTHER})
endif()

install(TARGETS makcu
    ARCHIVE DESTINATION lib
)

# Package configuration
set(CPACK_PACKAGE_NAME "makcu-cpp")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High-Performance C++ Library for MAKCU Mouse Controllers")
set(CPACK_PACKAGE_VENDOR "MAKCU")
set(CPACK_PACKAGE_CONTACT "support@makcu.com")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libudev1")
    set(CPACK_RPM_PACKAGE_REQUIRES "systemd-devel")
endif()

include(CPack)