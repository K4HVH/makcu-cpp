cmake_minimum_required(VERSION 3.16)

# Add executable for unit tests
add_executable(unit_tests unit_tests.cpp)

# Link against the MAKCU library
target_link_libraries(unit_tests makcu-static)

# Add test
add_test(NAME UnitTests COMMAND unit_tests)

# Set test properties
set_tests_properties(UnitTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Add integration tests if device is available
add_executable(integration_tests integration_tests.cpp)
target_link_libraries(integration_tests makcu-static)

# Integration tests require actual hardware
add_test(NAME IntegrationTests COMMAND integration_tests)
set_tests_properties(IntegrationTests PROPERTIES
    TIMEOUT 120
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    LABELS "hardware"
)

# Optional: Performance tests
if(BUILD_PERFORMANCE_TESTS)
    add_executable(performance_tests performance_tests.cpp)
    target_link_libraries(performance_tests makcu-static)
    
    add_test(NAME PerformanceTests COMMAND performance_tests)
    set_tests_properties(PerformanceTests PROPERTIES
        TIMEOUT 300
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        LABELS "performance"
    )
endif()
