@echo off
setlocal enabledelayedexpansion

REM Build script for makcu-cpp on Windows

echo === MAKCU C++ Windows Build Script ===
echo.

REM Check for required dependencies
echo Checking dependencies...
echo.

REM Check for CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake is not installed or not in PATH
    echo Download from: https://cmake.org/download/
    exit /b 1
)

REM Check for Visual Studio or Build Tools
where cl >nul 2>&1
if errorlevel 1 (
    echo [ERROR] MSVC compiler not found
    echo Please install Visual Studio or Visual Studio Build Tools
    echo Run this script from a Visual Studio Developer Command Prompt
    exit /b 1
)

echo [SUCCESS] All dependencies found!
echo.

REM Create build directory
echo Creating build directory...
if not exist build mkdir build
cd build

REM Run CMake configuration
echo Configuring project...
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo [ERROR] CMake configuration failed
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build . --config Release --parallel
if errorlevel 1 (
    echo [ERROR] Build failed
    exit /b 1
)

echo.
echo [SUCCESS] === Build completed successfully! ===
echo [INFO] Executable location: %CD%\bin\Release\makcu-cpp.exe

REM Test if the executable was created
if exist "bin\Release\makcu-cpp.exe" (
    echo [SUCCESS] ✓ Executable built successfully
) else (
    echo [ERROR] ✗ Executable not found
    exit /b 1
)

echo.
echo To run the demo:
echo cd build ^&^& bin\Release\makcu-cpp.exe