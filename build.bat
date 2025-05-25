@echo off
echo Building MAKCU C++ Library...
echo =============================

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
echo Building...
cmake --build . --config Release

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo.
echo Built executables:
echo - makcu-demo.exe (main demo)
echo - mouse-monitor.exe (mouse monitoring example)
echo - keyboard-automation.exe (keyboard automation example)
echo - mouse-automation.exe (mouse automation example)
echo - advanced-features.exe (advanced features demo)
echo - unit_tests.exe (unit tests)
echo - integration_tests.exe (integration tests - requires hardware)
echo.
echo For quick testing without CMake:
echo - Open makcu-basic-test.vcxproj in Visual Studio
echo - Build and run for basic functionality test
echo.
echo To run tests:
echo   ctest
echo.
echo To install (run as administrator):
echo   cmake --install . --config Release
echo.

pause
